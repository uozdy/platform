#include "TaskThreadPool.h"

namespace PLATFORM {

CTaskThreadPool::CTaskThreadPool()
{
	m_iCnt = 0;
	m_pThreads = NULL;
	m_bRun = false;
}

CTaskThreadPool::~CTaskThreadPool()
{
	
}

void CTaskThreadPool::StartSv(int n, int isDetached, int isSopeInSystem, int stackSize, const char* threadName)
{
	m_pThreads = (CThread**)malloc(sizeof(CThread*)*n);
	m_iCnt = n;
	m_bRun = true;
	for (int i = 0; i < n; i++)
	{
		m_pThreads[i] = new CThread();
		m_pThreads[i]->CreateThread(isDetached, isSopeInSystem, stackSize, CTaskThreadPool::TaskThread, this, threadName);
	}
}

void CTaskThreadPool::StopSv()
{
	if (!m_pThreads) return;
	m_bRun = false;
	m_tskCond.notify_all();
	
	for (int i = 0; i < m_iCnt; i++) {
		m_pThreads[i]->ExitThread();
		delete m_pThreads[i];
	}
	free(m_pThreads);
	m_pThreads = NULL;
}

void* CTaskThreadPool::TaskThread(void* lpParam)
{
	CTaskThreadPool* pThis = (CTaskThreadPool*)lpParam;
	
	Task task;
	while(pThis->m_bRun) {		
		{
			std::unique_lock<std::mutex> lock(pThis->m_tskMtx);
			pThis->m_tskCond.wait(lock, [pThis] {
				if (pThis->m_bRun) return !pThis->m_tskQue.empty();
				else return true;
			});

			if (!pThis->m_bRun) return NULL;
			task = std::move(pThis->m_tskQue.front());
			pThis->m_tskQue.pop();
		}
		task();
	}
	return NULL;
}

void CTaskThreadPool::Commit(Task task) {
	//lock
	{
		std::unique_lock<std::mutex> lock(m_tskMtx);
		m_tskQue.emplace(task);
	}
	m_tskCond.notify_one();
}

}
