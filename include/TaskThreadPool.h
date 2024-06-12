
#ifndef __TASKTTHREADPOOL_H__
#define __TASKTTHREADPOOL_H__

#include "Thread.h"
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <future>

namespace PLATFORM {

using Task = std::function<void()>;

class CTaskThreadPool
{
public:
	CTaskThreadPool();
	virtual ~CTaskThreadPool();
	void StartSv(int n, int isDetached, int isSopeInSystem, int stackSize, const char* threadName);
	void StopSv();
	void Commit(Task task);
	
protected:
	static void* TaskThread(void* lpParam);
private:
	bool				m_bRun;
	CThread**			m_pThreads;
	int					m_iCnt;
	std::queue<Task>		m_tskQue;
	std::mutex				m_tskMtx;
	std::condition_variable m_tskCond;
};

}
#endif