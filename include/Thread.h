#ifndef _TTHREAD_H__
#define _TTHREAD_H__
#include <thread>

namespace PLATFORM {
	typedef enum{
		THREADSTATE_NONE,
		THREADSTATE_CREATE,
		THREADSTATE_RUN,
		THREADSTATE_QUIT,
	}THREAD_STATE;
	
	typedef void* (*THREADFUNC)(void*);
	
	class CThread
	{
	public:
		CThread();
		~CThread();
	public:
		bool CreateThread(int isDetached, int isSopeInSystem, int stackSize, THREADFUNC func, void *context, const char* threadName);
		void ExitThread();
		bool IsThreadRun();
	protected:
		
	protected:
		THREAD_STATE		m_eRelease;
		THREADFUNC			m_pThreadFunc;
		void*				m_pContext;
		char				m_acName[20];
	};
}

#endif
