#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include "io_socket_ext.h"
#include "platform.h"

namespace PLATFORM {

static int __initsocket(int streamType, struct sockaddr* sin, int len)
{
	int iSocket = socket(sin->sa_family, streamType, 0);

	int optval = 1;
	setsockopt(iSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
	PLATFORM_SETNOBLOCK(iSocket);

	int err = bind(iSocket, sin, len);
	if (0 != err) {
		printf("bind err:%d errno:%d(%s)\n", err, errno, strerror(errno));
		closesocket(iSocket);
		return 0;
	}

	listen(iSocket, 30);
	return iSocket;
}

static int __connect(int socket, struct sockaddr* addr, int addrlen, int timeout)
{
	if (connect(socket, addr, addrlen) == -1) {
		int error = -1;
		int times = timeout / 10;
		fd_set set;
		struct timeval tm;
		while (true) {
			FD_ZERO(&set);
			FD_SET(socket, &set);
			tm.tv_sec = 0;
			tm.tv_usec = 10000;
			
			if (select(socket + 1, NULL, &set, NULL, &tm) > 0) {
				socklen_t len = sizeof(socklen_t);
				getsockopt(socket, SOL_SOCKET, SO_ERROR, (char *)&error, &len);
				if (error != 0) {
					if(!(error == EINTR || error == EAGAIN) ){
						return BYDATAPROC_SOCKET_ERROR;
					}
					if (times < 0) {
						return BYDATAPROC_SOCKETTIMEOUT;
					}
				}
				else
					break;
			}
			else if (times < 0) {
				return BYDATAPROC_SOCKETTIMEOUT;
			}

			times--;
		}
	}
	
	return BYDATAPROC_SOCKETSUCESS;
}

int io_socket_async_create(const char* pipe)
{
	struct sockaddr_un addr;
	memset( &addr, 0, sizeof(addr) );
	addr.sun_family  = AF_UNIX;
	__sk_addrun_init(&addr, pipe);
	return __initsocket(SOCK_STREAM, (struct sockaddr*)&addr, sizeof(addr));
}

int io_socket_async_create(bool pipe)
{
	int iSocket = socket(pipe?AF_UNIX:AF_INET, SOCK_STREAM, 0);

	int optval = 1;
	setsockopt(iSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
	PLATFORM_SETNOBLOCK(iSocket);
	return iSocket;
}

int io_socket_async_create(const char* ip, unsigned short port)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip?inet_addr(ip):INADDR_ANY;
	addr.sin_port = htons(port);
	return __initsocket(SOCK_STREAM, (struct sockaddr*)&addr, sizeof(addr));
}

int io_socket_async_create_udp(const char* ip, unsigned short port)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip?inet_addr(ip):INADDR_ANY;
	addr.sin_port = htons(port);
	return __initsocket(SOCK_DGRAM, (struct sockaddr*)&addr, sizeof(addr));
}

int io_socket_connect(int socket, const char* ip, unsigned short port, int timeout)
{
	sockaddr_in addrin;
	addrin.sin_family = AF_INET;
	addrin.sin_port = htons(port);
	addrin.sin_addr.s_addr = inet_addr(ip);
	if (addrin.sin_addr.s_addr == INADDR_NONE) {
		return BYDATAPROC_SOCKET_ERROR;
	}
	memset(&(addrin.sin_zero), '\0', 8);
	
	return __connect(socket, (struct sockaddr*)&addrin, sizeof(addrin), timeout);
}

int io_socket_pipe_connect(int socket, const char* pipe, int timeout)
{
	struct sockaddr_un addrU;
	memset(&addrU, 0, sizeof(addrU) );
	addrU.sun_family = AF_UNIX;
	__sk_addrun_init(&addrU, pipe);
	
	return __connect(socket, (struct sockaddr*)&addrU, sizeof(addrU), timeout);
}

void io_socket_async_accept(int sock, std::function<void(int)> fn)
{
	int remote;
	struct sockaddr_in addr = {0};
	socklen_t len = sizeof(addr);
	while ((remote = accept(sock, (struct sockaddr*)&addr, &len)) > 0) {  
		PLATFORM_SETNOBLOCK(remote);
		PLATFORM_SETLGER(remote);
		fn(remote);
	}
}

int io_socket_async_recv(int socket, char *buf, int n, int flags, unsigned int ms)
{
	fd_set fdr;
	struct timeval tm = {(long)ms/1000, (long)ms%1000*1000};
	FD_ZERO(&fdr);
	FD_SET(socket, &fdr);
	
	int ret = select(socket + 1, &fdr, NULL, NULL, &tm);
	if (ret > 0) {
		ret = recv(socket, buf, n, flags);
		if (0 == ret) {
			return BYDATAPROC_SOCKETSERVERCLOSE;
		}
		return ret;
	}else if (ret < 0) {
		return BYDATAPROC_SOCKETERROR;
	}
	return BYDATAPROC_SOCKET_NODATA;
}

int io_socket_async_recvfrom(int socket, char *buf, int n, int flags, struct sockaddr* addr, unsigned int ms)
{
	fd_set fdr;
	struct timeval tm = {(long)ms/1000, (long)ms%1000*1000};
	FD_ZERO(&fdr);
	FD_SET(socket, &fdr);
	
	int ret = select(socket + 1, &fdr, NULL, NULL, &tm);
	if (ret > 0) {
		socklen_t slen = sizeof(struct sockaddr_in);
		ret = recvfrom(socket, buf, n, flags, addr, &slen);
		if (0 == ret) {
			return BYDATAPROC_SOCKETSERVERCLOSE;
		}
		return ret;
	}else if (ret < 0) {
		return BYDATAPROC_SOCKETERROR;
	}
	return BYDATAPROC_SOCKET_NODATA;
}

int io_socket_sync_send(int socket, const char *buf, int n, int flags, unsigned int ms)
{
	fd_set fdr;
	int offset = 0, times = ms / 10;
	struct timeval	timeout;
	while (times-- > 0) {
		FD_ZERO(&fdr);
		FD_SET(socket, &fdr);
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;
		int ret = select(socket + 1, NULL, &fdr, NULL, &timeout);
		if (ret == 0) {
			continue;
		}else if (ret < 0) {
			return BYDATAPROC_SOCKET_ERROR;
		}else {
			int sendLen = (int)send(socket, buf+offset, n-offset, flags);
			if ((sendLen + offset) >= n) {
				return n;
			}else if (sendLen > 0) {
				offset += sendLen;
			}else {
				if (errno == EINTR || errno == EAGAIN) {
					continue;
				}else {
					return BYDATAPROC_SOCKET_ERROR;
				}	
			}
		}
	}
	
	return BYDATAPROC_SOCKETTIMEOUT;
}

std::string io_socket_addr2str(struct sockaddr* addr, unsigned int addrlen)
{
	char hoststr[64];
	char portstr[64];
	getnameinfo(addr,  addrlen, 
		hoststr, sizeof(hoststr), 
		portstr, sizeof(portstr), 
		NI_NUMERICHOST | NI_NUMERICSERV );
	
	std::string str = hoststr;
	str += ":";
	str += portstr;
	return str;
}

int io_socket_bindDevices(int sk, const char* devName)
{
    struct ifreq nif;
    strcpy(nif.ifr_name, devName);
	if (setsockopt(sk, SOL_SOCKET, SO_BINDTODEVICE, (char *)&nif, sizeof(nif)) < 0)
    {
        printf("bind interface(%s) fail, errno: %d \r\n", devName, errno);
		return -1;		
    }
	return 0;
}

std::vector<struct sockaddr_in> io_socket_get_broadcast(unsigned short port)
{
	return __get_broadcast_addr(port);
}

}
