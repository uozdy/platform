#include "Thread.h"
#include <string.h>
#include "platform.h"
namespace PLATFORM
{
	CThread::CThread()
		: m_eRelease(THREADSTATE_NONE)
		, m_pThreadFunc(NULL)
		, m_pContext(NULL) 
		
	{
		memset(m_acName, 0x00, sizeof(m_acName));
	}

	CThread::~CThread()
	{
		m_pContext = NULL;
	}

	bool CThread::CreateThread(int isDetached, int isScopeInSystem, int stackSize, THREADFUNC func, void *context, const char* threadName)
	{
		if (func == NULL) return false;
        if (THREADSTATE_NONE != m_eRelease) return false;
		
		m_pContext = context;
		m_pThreadFunc = func;
		
		strncpy(m_acName, threadName, 20);
		
		m_eRelease = THREADSTATE_CREATE;
		std::thread thread([this] {
			m_eRelease = THREADSTATE_RUN;
			m_pThreadFunc(m_pContext);
			m_eRelease = THREADSTATE_QUIT;
		});
		thread.detach();
		return true;
	}

	void CThread::ExitThread()
	{
		while (m_eRelease != THREADSTATE_NONE) {
			if (m_eRelease == THREADSTATE_QUIT) {
				break;
			}
			Sleep(10);
		}
		m_eRelease = THREADSTATE_NONE;
	}

	bool CThread::IsThreadRun()
	{
		return (bool)(m_eRelease != THREADSTATE_NONE && m_eRelease != THREADSTATE_QUIT);
	}
}


