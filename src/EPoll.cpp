#include <stdlib.h>
#include "EPoll.h"
#include "platform.h"

namespace PLATFORM
{

CEPoll::CEPoll()
	: m_iEpoll(0)
{
}

CEPoll::~CEPoll()
{
}

int CEPoll::create(int size)
{
	int localid = epoll_create(size);
	if (localid < 0){
		printf("epoll_create err %d\n", localid);
		return localid;
	}
		
	m_iEpoll = localid;
	return 0;
}

int CEPoll::release()
{
	if (m_iEpoll > 0) {
		epoll_release(m_iEpoll);
		m_iEpoll = 0;
    }
	return 0;
}

int CEPoll::addSock(const int& sock, void* events)
{
	if (NULL == events || sock <= 0 || m_iEpoll <= 0)
		return -8000;
	
	int ret = ::epoll_ctl(m_iEpoll, EPOLL_CTL_ADD, sock, (struct epoll_event*)events);
	if (ret < 0){
		printf("epoll_ctl ADD err\n");
		return ret;
	}
	return ret;
}

int CEPoll::delSock(const int& sock)
{
	if (sock <= 0 || m_iEpoll <= 0)
		return -8000;
	
	int ret = ::epoll_ctl(m_iEpoll, EPOLL_CTL_DEL, sock, NULL);
	if (ret < 0){
		printf("epoll_ctl DEL err\n");
		return ret;
	}
	
	return ret;
}

int CEPoll::changeEvent(const int& sock, void* events)
{
	int ret = 0;
	if (sock <= 0 || m_iEpoll <= 0) {
		printf("CEPoll uninit\n");
		return -8000;
	}

	ret = ::epoll_ctl(m_iEpoll, EPOLL_CTL_MOD, sock, (struct epoll_event*)events);
	if (ret < 0){
		printf("epoll_ctl Mod err\n");
		return ret;
	}
	return ret;
}

int CEPoll::wait(void* events, int maxevents, int tm)
{
	return ::epoll_wait(m_iEpoll, (struct epoll_event*)events, maxevents, tm);
}

}
