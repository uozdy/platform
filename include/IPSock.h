/*
* Copyright (c) 2019-2020,微瞳科技 研发体系
* All rights reserved
*
* 文件名称： ISock.h
* 文件标识：
* 摘 要：
*
* 当前版本：1.0
* 作 者：luoc
* 完成日期：
*
* 取代版本：
* 原作者 ：
* 完成日期：
*/
#ifndef __WTMEDIASDK_SRC_COMMON_COMPONENT_DEBUG_NET_IPSOCK__
#define __WTMEDIASDK_SRC_COMMON_COMPONENT_DEBUG_NET_IPSOCK__
namespace PLATFORM
{
	class IParse;
	class IPSock
	{
	public:
		virtual ~IPSock() {  }
	
		virtual bool IsQuickQuit() = 0;
		virtual void QuickQuit(bool bQuit) = 0;
		
		virtual int GetSocket() = 0;
		virtual void SetSocket(int sock) = 0;
		
		virtual IParse* GetParse() = 0;
		virtual void SetParse(IParse* pParse) = 0;
		
		virtual char* GetRecvBuf(int& recvLen, int &maxSize) = 0;
		virtual void SetRecvLen(int recvLen) = 0;
	};
}
#endif
