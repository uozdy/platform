/*
* Copyright (c) 2016,jiancheng 研发体系
* All rights reserved.
*
* 文件名称：platform.h
* 文件标识：
* 摘 要：跨平台类型与函数重定义
*
* 当前版本：1.0
* 作 者：luoc
* 完成日期：2023年9月8日
*/
#ifndef __SRC_PLATFORM_H__
#define __SRC_PLATFORM_H__

#define PLATFORM_SETLGER(s)		{linger lger = {1,0};setsockopt(s, 0, SO_LINGER, (const char*)&lger, sizeof(linger));}

#ifndef WIN32

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h> 
#include <sys/un.h>
#include <net/if.h>
#include <netdb.h>
#include <unistd.h>
#include <strings.h>

#define GetCurrentProcessId()		getpid()
#define PLATFORM_SETNOBLOCK(s)		{int opts = fcntl(s, F_GETFL);fcntl(s, F_SETFL, opts | O_NONBLOCK);}
void __sk_addrun_init(struct sockaddr_un* addr, const char* pipe);
int closesocket(int sk);

bool __x_isdir(const char* path);
int __x_removedir(const char *dir);
void __x_copydir(const char * sourcePath, const char * destPath);

#else

#include <stdint.h>
#include <string.h>
#include <winsock2.h>
#include <io.h>
#include <vector>

#define PLATFORM_SETNOBLOCK(s)		{unsigned long nonBlock = 1;ioctlsocket(s, FIONBIO, &nonBlock);}

typedef unsigned int key_t;
#ifdef __cplusplus
extern "C" {
#endif

#define STDOUT_FILENO	STD_OUTPUT_HANDLE
#define STDERR_FILENO	STD_ERROR_HANDLE
#define STDIN_FILENO	STD_INPUT_HANDLE
//#define STDOUT_FILENO	GetStdHandle(STD_OUTPUT_HANDLE)
//#define STDERR_FILENO	GetStdHandle(STD_ERROR_HANDLE)
//#define STDIN_FILENO	GetStdHandle(STD_INPUT_HANDLE)

typedef int socklen_t;

//sys/socket.h
#ifndef O_NONBLOCK
#define O_NONBLOCK	  		04000
#endif

#define SO_BINDTODEVICE		25
# define NI_NUMERICHOST		1
# define NI_NUMERICSERV 	2
# define NI_NOFQDN			4
# define NI_NAMEREQD		8
# define NI_DGRAM			16

struct sockaddr_un
{
	short   sun_family;
	u_short sin_port;
	struct  in_addr sin_addr;
	char    sin_zero[8];
};

#define IFNAMSIZ 	16
struct ifmap
{
	unsigned long mem_start;
	unsigned long mem_end;
	unsigned short base_addr;
	unsigned char irq;
	unsigned char dma;
	unsigned char port;
};

struct ifreq
{
	char ifr_name[IFNAMSIZ]; /* Interface name */
	union {
		struct sockaddr ifr_addr;
		struct sockaddr ifr_dstaddr;
		struct sockaddr ifr_broadaddr;
		struct sockaddr ifr_netmask;
		struct sockaddr ifr_hwaddr;
		short ifr_flags;
		int ifr_ifindex;
		int ifr_metric;
		int ifr_mtu;
		struct ifmap ifr_map;
		char ifr_slave[IFNAMSIZ];
		char ifr_newname[IFNAMSIZ];
		char* ifr_data;
	};
};


//sys/epoll.h
#define EPOLLIN 			0x001
#define EPOLLPRI 			0x002
#define EPOLLOUT 			0x004
#define EPOLLRDNORM 		0x040
#define EPOLLRDBAND 		0x080
#define EPOLLWRNORM 		0x100
#define EPOLLWRBAND 		0x200
#define EPOLLMSG 			0x400
#define EPOLLERR 			0x008
#define EPOLLHUP 			0x010
#define EPOLLRDHUP 			0x2000
#define EPOLLEXCLUSIVE 		(1u << 28)
#define EPOLLWAKEUP 		(1u << 29)
#define EPOLLONESHOT 		(1u << 30)
#define EPOLLET 			(1u << 31)

#define EPOLL_CTL_ADD 		1
#define EPOLL_CTL_DEL 		2
#define EPOLL_CTL_MOD 		3

typedef struct epoll_data
{
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event
{
    uint32_t events;
    epoll_data_t data;
	
	bool operator==(int fd) {
		return data.fd == fd;
	}
};

int epoll_create(int size);
int epoll_ctl(int epfd, int cmd, int fd, struct epoll_event* event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
int epoll_release(int epfd);

//sys/ipc.h
//sys/msg.h
#define IPC_CREAT	01000
#define IPC_EXCL	02000
#define IPC_NOWAIT	04000
#define IPC_RMID	0
#define IPC_SET		1
#define IPC_STAT	2
#define IPC_INFO	3
#define IPC_PRIVATE	((__key_t) 0)

int msgget(key_t __key, int __msgflg);
int msgsnd (int msqid, const void *msgp, size_t msgsz, int msgflg);	
int msgrcv (int msqid, void *msgp, size_t msgsz, long int msgtyp, int msgflg);

int getuid(void);
int getnameinfo (const struct sockaddr *sa, socklen_t addrlen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags);
void __sk_addrun_init(struct sockaddr_un* addr, const char* pipe);

#ifndef _TIMEZONE_DEFINED
#define _TIMEZONE_DEFINED
struct timezone
{
	int tz_minuteswest;
	int tz_dsttime;
};
#endif
int gettimeofday(struct timeval *tp, struct timezone *tzp);

#define strcasecmp		stricmp

bool __x_isdir(const char* path);
int __x_removedir(const char *dir);
void __x_copydir(const char * sourcePath, const char * destPath);

#ifdef __cplusplus
}
#endif

std::vector<struct sockaddr_in> __get_broadcast_addr(unsigned short port);

#endif
#endif