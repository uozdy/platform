#ifndef __PLATFORM_IPARSE__
#define __PLATFORM_IPARSE__
#include <stdio.h>
#include <sys/types.h>
#ifdef WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif

namespace PLATFORM {

	enum DATACHECK
	{
		DATAERROR,
		DATAOK_NOKEY,
		DATAOK_WITHKEY,
		DATANOCOMP,
		HEADNOCOMP,
	};

	class IParse 
	{
	public: //require methed
		virtual int onParseMsg(int fd, char* pData, int len, struct sockaddr* addr = NULL) = 0;
		virtual char* GetBufInfo(int* unparselen, int* maxlen) = 0;
		
		
	public: //option methed
		virtual bool IsQuit(){return false;};
		virtual int onCheckData(char* pHead, int len){return DATAOK_NOKEY;};
		virtual int onDataNotComp(char* pHead, int len, int& RecvBufLen){RecvBufLen=0;return 0;};
		virtual int onParseKey(int fd, char* pData, int len, struct sockaddr* addr = NULL){return 0;};
		virtual void SetUnParseLen(int unparselen) {};

	public: //option methed
		virtual bool WriteData(int fd){return false;}
	};
}

#endif