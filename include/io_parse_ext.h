#ifndef __IO_PARSE_EXT_H__
#define __IO_PARSE_EXT_H__
#include "IParse.h"

namespace PLATFORM {
	typedef int (*io_parse_recv_call)(int fd, char* pRecvBuf, int iMaxSize, struct sockaddr* addr);
	
	int  io_parse_data(int fd, IParse* pParse, char* pBuf, int& unparselen, struct sockaddr* addr);
	int io_parse_pkt(int fd, IParse* pParse, char* pBuf, int unparselen, struct sockaddr* addr);
	bool io_parse_recv(int fd, IParse* pParse, io_parse_recv_call fun, struct sockaddr* addr, bool bBlock);
}
#endif