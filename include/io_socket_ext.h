#ifndef __SOCKET_EXT_H__
#define __SOCKET_EXT_H__
#include <vector>
#include <functional>
#include <condition_variable>
#ifdef WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif

namespace PLATFORM {
	
typedef enum
{
	BYDATAPROC_SOCKETUSERCANCEL = -0x8,
	BYDATAPROC_SOCKETIPERROR = -0x7,
	BYDATAPROC_SOCKETSERVERCLOSE = -0x5,
	BYDATAPROC_SOCKETTIMEOUT = -0x4,
	BYDATAPROC_SOCKETERROR = -0x3,
	BYDATAPROC_SOCKET_CLOSE = -0x2,
	BYDATAPROC_SOCKET_ERROR,
	BYDATAPROC_SOCKET_NODATA,
	BYDATAPROC_SOCKETSUCESS = BYDATAPROC_SOCKET_NODATA,
}SOCKET_STATA;

int io_socket_async_create(bool pipe);
int io_socket_async_create(const char* pipe);
int io_socket_async_create(const char* ip, unsigned short port);
int io_socket_async_create_udp(const char* ip, unsigned short port);

int io_socket_connect(int socket, const char* ip, unsigned short port, int timeout);
int io_socket_pipe_connect(int socket, const char* pipe, int timeout);

void io_socket_async_accept(int sock, std::function<void(int)>);

int io_socket_async_recv(int socket, char *buf, int n, int flags, unsigned int ms);
int io_socket_async_recvfrom(int socket, char *buf, int n, int flags, struct sockaddr* addr, unsigned int ms);

int io_socket_sync_send(int socket, const char *buf, int n, int flags, unsigned int ms);

std::string io_socket_addr2str(struct sockaddr* addr, unsigned int addrlen);
int io_socket_bindDevices(int sk, const char* devName);

std::vector<struct sockaddr_in> io_socket_get_broadcast(unsigned short port);

}
#endif
