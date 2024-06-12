
#ifndef __MSGTHREADPOOL_H__
#define __MSGTHREADPOOL_H__

#include "Thread.h"
#include "MsgQue.hpp"
#include <stdlib.h>
namespace PLATFORM {

template<typename T, typename V>
class CMsgThreadPool : public CMsgQue<T, V>
{
public:
	CMsgThreadPool();
	virtual ~CMsgThreadPool();
	void StartSv(int n, int isDetached, int isSopeInSystem, int stackSize, THREADFUNC pFunc, void* param, const char* threadName);
	void StopSv();

private:
	CThread**		m_pThreads;
	int				m_iCnt;
};

template<typename T, typename V>
CMsgThreadPool<T, V>::CMsgThreadPool()
{
	m_iCnt = 0;
	m_pThreads = NULL;
}

template<typename T, typename V>
CMsgThreadPool<T, V>::~CMsgThreadPool()
{
	StopSv();
}

template<typename T, typename V>
void CMsgThreadPool<T, V>::StartSv(int n, int isDetached, int isSopeInSystem, int stackSize, THREADFUNC pFunc, void* param, const char* threadName)
{
	m_pThreads = (CThread**)malloc(sizeof(CThread*)*n);
	m_iCnt = n;
	for (int i = 0; i < n; i++) {
		m_pThreads[i] = new CThread();
		m_pThreads[i]->CreateThread(isDetached, isSopeInSystem, stackSize, pFunc, param, threadName);
	}
}

template<typename T, typename V>
void CMsgThreadPool<T, V>::StopSv()
{
	if (!m_pThreads) return;
	
	for (int i = 0; i < m_iCnt; i++) {
		m_pThreads[i]->ExitThread();
		delete m_pThreads[i];
	}
	free(m_pThreads);
	m_pThreads = NULL;
}

}
#endif