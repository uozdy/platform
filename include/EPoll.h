#ifndef __WTMEDIASDK_SRC_PLATFORM_SRC_NET_EPOLL__
#define __WTMEDIASDK_SRC_PLATFORM_SRC_NET_EPOLL__
#include <stdio.h>

namespace PLATFORM
{
class CEPoll
{
public:
	CEPoll();
	~CEPoll();

public: 
	int create(int size);
	int addSock(const int& sock, void* events);
	int changeEvent(const int& sock, void* events);
	int delSock(const int& sock);

	int wait(void* events, int maxevents, int timeout);
	int release();
private:
	int 	m_iEpoll;
};
}

#endif
