#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <vector>

#include "net_udp_loop.h"
#include "io_socket_ext.h"
#include "io_parse_ext.h"
#include "platform.h"

namespace PLATFORM
{
	#define DISCOVERY_MAX_LEN		1460
	#define MAX_LISTEN				32
	
	typedef struct tagNetUdpLoopEvent {
		int fd;
		int times;
		IParse* pParser;
	}NetUdpLoopEvent;
	static std::vector<NetUdpLoopEvent*>	gEvent;

	CNetUdpLoop* CNetUdpLoop::s_pInstance = NULL;
	CNetUdpLoop* CNetUdpLoop::getInstance() {
		if (s_pInstance == NULL) s_pInstance = new CNetUdpLoop();
		return s_pInstance;
	}

	void CNetUdpLoop::releaseInstance() {
		if (s_pInstance == NULL) return;
		s_pInstance->StopThread();
		delete s_pInstance;
		s_pInstance = NULL;
	}
	
	int CNetUdpLoop::Listen(int port, IParse* pParser)
	{
		int broadcast = 1;
		int iSocket = io_socket_async_create_udp(NULL, port);
		if (iSocket > 0) {
			setsockopt(iSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof (broadcast));
		
			NetUdpLoopEvent* ev = new NetUdpLoopEvent;
			ev->fd = iSocket;
			ev->times = 0;
			ev->pParser = pParser;
			gEvent.push_back(ev);
			
			struct epoll_event event = {0};
			event.data.fd = iSocket;
			event.data.ptr = ev;
			event.events = EPOLLET | EPOLLIN;
			m_ePoll.addSock(iSocket, &event);
		}
		
		return iSocket;
	}
	
	int CNetUdpLoop::Run()
	{
		m_bThreadRun = true;
		
		discoverThread(this);
		return 0;
	}
	
	int CNetUdpLoop::RunInNewThread()
	{
		if (m_bThreadRun) return -1;
		
		m_bThreadRun = true;
		m_hThread.CreateThread(1, 1, 512*1024, discoverThread, this, "discoverthread");
		return 0;
	}
	
	void CNetUdpLoop::StopThread()
	{
		m_bThreadRun = false;
		m_hThread.ExitThread();
	}
	
	void* CNetUdpLoop::discoverThread(void* param)
	{
		CNetUdpLoop* self = (CNetUdpLoop*)param;
		struct sockaddr_in fromAddr;
		char rcvBuf[DISCOVERY_MAX_LEN];
		struct epoll_event  aEvents[MAX_LISTEN] = {0};
		while (self->m_bThreadRun) {
			int cnt = self->m_ePoll.wait(aEvents, MAX_LISTEN, 1000);
			for ( int i = 0; i < cnt; i++ ) {
				if (aEvents[i].events & EPOLLIN) {
					NetUdpLoopEvent* ev = (NetUdpLoopEvent*)aEvents[i].data.ptr;
					int rcvLen = io_socket_async_recvfrom(ev->fd, rcvBuf, DISCOVERY_MAX_LEN, 0, (struct sockaddr*)&fromAddr, 1000);
					if (rcvLen > 0) {
						io_parse_pkt(ev->fd, ev->pParser, rcvBuf, rcvLen, (struct sockaddr*)&fromAddr);
						ev->times = 3;
					}
				}
			}
			
			std::vector<NetUdpLoopEvent*>::iterator it = gEvent.begin();
			for (; it != gEvent.end(); ++it) {
				NetUdpLoopEvent* ev = (NetUdpLoopEvent*)*it;
				if ( --ev->times <= 0) {
					ev->times = 3;
					ev->pParser->WriteData(ev->fd);
				}
			}
		}
		return NULL;
	}
	
	CNetUdpLoop::CNetUdpLoop()
		: m_bThreadRun(false)
	{
		m_ePoll.create(MAX_LISTEN);
	}
	
	CNetUdpLoop::~CNetUdpLoop()
	{
		StopThread();
		std::vector<NetUdpLoopEvent*>::iterator it = gEvent.begin();
		while (it != gEvent.end()) {
			NetUdpLoopEvent* ev = (NetUdpLoopEvent*)*it;
			closesocket(ev->fd);
			delete ev;
			it = gEvent.erase(it);
		}
	}	
}
