/*
* Copyright (c) 2019-2020,΢ͫ�Ƽ� �з���ϵ
* All rights reserved
*
* �ļ����ƣ� ISock.h
* �ļ���ʶ��
* ժ Ҫ��
*
* ��ǰ�汾��1.0
* �� �ߣ�luoc
* ������ڣ�
*
* ȡ���汾��
* ԭ���� ��
* ������ڣ�
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
