#ifndef DISCOVERY_H_
#define DISCOVERY_H_

#include <stdint.h>
#include "Thread.h"
#include "IParse.h"
#include "EPoll.h"

namespace PLATFORM
{

class CNetUdpLoop {
public:
	static CNetUdpLoop* getInstance();
	static void releaseInstance();
	int Listen(int port, IParse* pParser);

	int Run();
	int RunInNewThread();
	void StopThread();
	
protected:
	static void* discoverThread(void*);
	
private:
	CNetUdpLoop();
	~CNetUdpLoop();
	
private:
	static CNetUdpLoop*		s_pInstance;
	CEPoll			m_ePoll;
	CThread			m_hThread;
	bool			m_bThreadRun;
};

}
#endif 
