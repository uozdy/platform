/*
* Copyright (c) 2016,bangyan 研发体系
* All rights reserved.
*
* 文件名称：MsgQue.hpp
* 文件标识：
* 摘 要：消息队列
*
* 当前版本：1.0
* 作 者：luoc
* 完成日期：2016年11月8日
*/
#ifndef __BYMEDIASDK_SRC_PLATFORM_MSGQUE__
#define __BYMEDIASDK_SRC_PLATFORM_MSGQUE__
#include <mutex>
#include <condition_variable>

//bool operator ==(const struct tagTASKMSG& msg) const {
//	return msg.pUsrPtr == pUsrPtr;
//}

namespace PLATFORM{
	template<typename T, typename V>
	class CMsgQue
	{
	public:
		CMsgQue();
		virtual ~CMsgQue();
	public:
		void PostMsg(T& msg);
		bool PeekMsg(T& msg);
		void PeekEndMsg(T& msg, void (*pmsgfun)(T& msg));
		void ClearMsg();
		
		void Wait(); 
		void NotifyMsg(T& msg);
	private:
		V       	    m_vMsg;
		std::mutex 		m_msgMtx;
		std::condition_variable m_msgCond;
	};

	template<typename T, typename V>
	CMsgQue<T, V>::CMsgQue()
	{
	}

	template<typename T, typename V>
	CMsgQue<T, V>::~CMsgQue()
	{
	}

	template<typename T, typename V>
	void CMsgQue<T, V>::PostMsg(T& msg)
	{
		std::unique_lock<std::mutex> lock(m_msgMtx);
		m_vMsg.push_back(msg);
	}

	template<typename T, typename V>
	void CMsgQue<T, V>::NotifyMsg(T& msg)
	{
		std::unique_lock<std::mutex> lock(m_msgMtx);
		m_vMsg.push_back(msg);
		m_msgCond.notify_one();
	}

	template<typename T, typename V>
	void CMsgQue<T, V>::ClearMsg()
	{
		std::unique_lock<std::mutex> lock(m_msgMtx);
		m_vMsg.clear();
	}

	template<typename T, typename V>
	bool CMsgQue<T, V>::PeekMsg(T& msg)
	{
		std::unique_lock<std::mutex> lock(m_msgMtx);
		unsigned long size = m_vMsg.size();
		if (size > 0) {
			msg = m_vMsg[0];
			m_vMsg.erase(m_vMsg.begin());
			return true;
		}
		return false;
	}


	template<typename T, typename V>
	void CMsgQue<T, V>::PeekEndMsg(T& msg, void (*pmsgfun)(T& msg))
	{
		typename V::iterator it;
		std::unique_lock<std::mutex> lock(m_msgMtx);
		for (it = m_vMsg.begin(); it < m_vMsg.end();) {
			if (msg == *it) {
				msg = *it;
				it = m_vMsg.erase(it);
				pmsgfun(msg);
			}else {
				++it;
			}
		}
	}

	template<typename T, typename V>
	void CMsgQue<T, V>::Wait() 
	{
		std::unique_lock<std::mutex> lock(m_msgMtx);
		m_msgCond.wait(lock, [this]() {
			return !m_vMsg.empty();
		});
	}
	
}
#endif