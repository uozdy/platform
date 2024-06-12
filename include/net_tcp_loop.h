#ifndef __PLATFORM_SRC_NET_SVRLOGIC__
#define __PLATFORM_SRC_NET_SVRLOGIC__
#include <vector>

#include "EPoll.h"
#include "Thread.h"
#include "IParse.h"

namespace PLATFORM
{
	enum{
		ENETMSG_ONACCEPT,
		ENETMSG_ONRELEASE,
	};
	typedef void (*OnNetEventFunc)(int fd, int msg, void* wParam, void* lParam);
	typedef int  (*OnNetRecvFunc)(int socket, char* pRecvBuf, int iMaxSize, struct sockaddr* addr);

	class CNetTcpLoop
	{
	public:
		CNetTcpLoop();
		virtual ~CNetTcpLoop();
		void* AddSvr(int sock, OnNetEventFunc onEvent, void* pUserPtr);
		void* AddClt(int sock, OnNetEventFunc onEvent, OnNetRecvFunc onRecv, IParse* pParase, void* pUserPtr);
		void* ChgClt(int sock, void* pEventPtr, bool bWrite);
		
		void RunOnce(int ms);
		CThread* RunInNewThread(int ms, bool* bRun);

	private:
		void __runLogic(void* aEvents, int ms);

	private:
		CEPoll				m_ePoll;
		std::vector<void*>	m_pVtSvrSk;
	};
}

#endif