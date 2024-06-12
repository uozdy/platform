#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include "net_tcp_loop.h"
#include "io_parse_ext.h"
#include "io_socket_ext.h"
#include "platform.h"

namespace PLATFORM
{
	#define MAX_LISTEN			32
	#define EVTPTRTYPE_SERVER  	0
	#define EVTPTRTYPE_CLIENT  	1
	#define PLATFORM_SAFACE_DELETE(obj)	if(obj){delete obj;obj=NULL;}
	
	typedef struct{
		unsigned short		type;			// [0: server  1:client]
		int					iSocket;
		OnNetEventFunc		onEvent;
		io_parse_recv_call	recvfunc;
		IParse*				pParse;
		void*				pUserPtr;
	}EventPtr;
	
	CNetTcpLoop::CNetTcpLoop()
	{
		m_ePoll.create(MAX_LISTEN);
	}
	
	CNetTcpLoop::~CNetTcpLoop()
	{
		std::vector<void*>::iterator it = m_pVtSvrSk.begin();
		for(; it != m_pVtSvrSk.end(); ){
			EventPtr* pPtr = (EventPtr*)*it;
			assert(pPtr);
			assert(pPtr->iSocket > 0);
			
			PLATFORM_SAFACE_DELETE(pPtr);
			it = m_pVtSvrSk.erase(it);
		}
		
		m_ePoll.release();
	}
	
	void* CNetTcpLoop::AddSvr(int sock, OnNetEventFunc onEvent, void* pUserPtr)
	{
		struct epoll_event event;
		EventPtr* pPtr = new EventPtr;
		
		pPtr->iSocket = sock;
		pPtr->type = EVTPTRTYPE_SERVER;
		pPtr->onEvent = onEvent;
		pPtr->recvfunc = NULL;
		pPtr->pParse = NULL;
		pPtr->pUserPtr = pUserPtr;
		
		event.data.fd = sock;
		event.data.ptr = pPtr;
		event.events = EPOLLET | EPOLLIN;
		m_ePoll.addSock(sock, &event);
		
		m_pVtSvrSk.push_back(pPtr);
		return pPtr;
	}
	
	void* CNetTcpLoop::AddClt(int sock, OnNetEventFunc onEvent, OnNetRecvFunc onRecv, IParse* pParase, void* pUserPtr)
	{
		EventPtr* pPtr = new EventPtr;
		pPtr->iSocket = sock;
		pPtr->type = EVTPTRTYPE_CLIENT;
		pPtr->onEvent = onEvent;
		pPtr->recvfunc = onRecv;
		pPtr->pParse = pParase;
		pPtr->pUserPtr = pUserPtr;
		
		struct epoll_event event;
		event.data.fd = sock;
		event.data.ptr = pPtr;
		event.events = EPOLLET | EPOLLIN;
		m_ePoll.addSock(sock, &event);
		
		return pPtr;
	}

	void* CNetTcpLoop::ChgClt(int sock, void* pEventPtr, bool bWrite)
	{		
		struct epoll_event event;
		event.data.ptr = pEventPtr;
		event.events = EPOLLET | EPOLLIN | (bWrite?EPOLLOUT:0);
		m_ePoll.changeEvent(sock, &event);

		return pEventPtr;
	}

	void CNetTcpLoop::RunOnce(int ms)
	{
		struct epoll_event  aEvents[MAX_LISTEN];
		__runLogic(aEvents, ms);
	}

	CThread* CNetTcpLoop::RunInNewThread(int ms, bool* bRun)
	{
		CThread* pThread = new CThread();
		struct NetTcpLoopTmpSt 
		{
			int				ms;
			bool*			bRun;
			CNetTcpLoop*	pLoop;

			NetTcpLoopTmpSt(int _ms, bool* _bRun, CNetTcpLoop* _pLoop) {
				ms = _ms;
				bRun = _bRun;
				pLoop = _pLoop;
			}
		};
		NetTcpLoopTmpSt* pSt = new NetTcpLoopTmpSt(ms, bRun, this);
		pThread->CreateThread(1, 1, 512 * 1024, [](void* param) -> void* {
			NetTcpLoopTmpSt* pSt = (NetTcpLoopTmpSt*)param;
			struct epoll_event  aEvents[MAX_LISTEN];
			while (*pSt->bRun) {
				pSt->pLoop->__runLogic(aEvents, pSt->ms);
			}
			delete pSt;
			return NULL;
		}, pSt, "tcploop2");
		return pThread;
	}

	void CNetTcpLoop::__runLogic(void* aEvents, int ms)
	{
		int cnt = m_ePoll.wait(aEvents, MAX_LISTEN, ms);
		for (int i = 0; i < cnt; i++) {
			EventPtr* pPtr = (EventPtr*)((struct epoll_event*)aEvents)[i].data.ptr;
			assert(pPtr->iSocket > 0);

			if (EVTPTRTYPE_SERVER == pPtr->type) {
				io_socket_async_accept(pPtr->iSocket, [&](int remote) {
					pPtr->onEvent(remote, ENETMSG_ONACCEPT, NULL, pPtr->pUserPtr);
				});
			}
			else {
				if (((struct epoll_event*)aEvents)[i].events & EPOLLIN) {
					struct sockaddr_in addr;
					if (!io_parse_recv(pPtr->iSocket, pPtr->pParse, pPtr->recvfunc, (struct sockaddr*)&addr, true)) {
						m_ePoll.delSock(pPtr->iSocket);
						pPtr->onEvent(pPtr->iSocket, ENETMSG_ONRELEASE, pPtr->pParse, pPtr->pUserPtr);
						PLATFORM_SAFACE_DELETE(pPtr);
					}
				}
				if (((struct epoll_event*)aEvents)[i].events & EPOLLOUT) {
					if (!pPtr->pParse->WriteData(pPtr->iSocket)) {
						m_ePoll.delSock(pPtr->iSocket);
						pPtr->onEvent(pPtr->iSocket, ENETMSG_ONRELEASE, pPtr->pParse, pPtr->pUserPtr);
						PLATFORM_SAFACE_DELETE(pPtr);
					}
				}
			}
		}
	}
}
