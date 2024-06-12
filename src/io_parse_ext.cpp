#include <string.h>
#include <errno.h>
#include "io_parse_ext.h"

namespace PLATFORM {
	
int io_parse_data(int fd, IParse* pParse, char* pBuf, int& unparselen, struct sockaddr* addr)
{
	int parsePoint = 0;
	while (parsePoint < unparselen)
	{
		int rsLen = unparselen - parsePoint;
		switch (pParse->onCheckData(pBuf + parsePoint, rsLen))
		{
		case DATAERROR:
			parsePoint++;
			continue;
		case DATAOK_NOKEY: {
			int r = pParse->onParseMsg(fd, pBuf + parsePoint, rsLen, addr);
			if (r == 0) break;
			parsePoint += r;
			continue;
		}			
		case DATAOK_WITHKEY:
		{
			int r = pParse->onParseKey(fd, pBuf + parsePoint, rsLen, addr);				
			if (r == 0) {
				int t = pParse->onParseMsg(fd, pBuf + parsePoint, rsLen, addr);
				if (t == 0) break;
				parsePoint += t;
			}else {
				parsePoint += r;
			}
			continue;
		}
		case HEADNOCOMP:
			break;
		case DATANOCOMP:
			pParse->onDataNotComp(pBuf + parsePoint, unparselen - parsePoint, unparselen);
			if (unparselen == 0) parsePoint = 0;
			break;
		}
		break;
	}
	
	if (parsePoint > 0) {
		if (unparselen - parsePoint > 0){
			memmove(pBuf, pBuf + parsePoint, unparselen - parsePoint);
			unparselen -= parsePoint;
		}else{
			unparselen = 0;
		}
		pParse->SetUnParseLen(unparselen);
	}
	return parsePoint;
}

bool io_parse_recv(int fd, IParse* pParse, io_parse_recv_call fun, struct sockaddr* addr, bool bBlock)
{
	bool rzRet = false;
	int maxlen, unparselen;
	char* pBuf = pParse->GetBufInfo(&unparselen, &maxlen);
	
	do
	{
		int ret = (fun)(fd, pBuf+unparselen, maxlen-unparselen, addr);
		if (ret == 0) {
			return rzRet;
		}
		if (ret < 0) {
			if (errno == EINTR) {
				continue;
			}else if (errno == EAGAIN) {
				break;
			}else {
				return false;
			}
		}
		
		unparselen += ret;
		pParse->SetUnParseLen(unparselen);
		io_parse_data(fd, pParse, pBuf, unparselen, addr);
		if (pParse->IsQuit()) {
			return false;
		}
		return true;
	}while (bBlock);
	return true;
}

int io_parse_pkt(int fd, IParse* pParse, char* pBuf, int unparselen, struct sockaddr* addr)
{
	switch (pParse->onCheckData(pBuf, unparselen))
	{
	case DATAOK_NOKEY: {
		return pParse->onParseMsg(fd, pBuf, unparselen, addr);
	}			
	case DATAOK_WITHKEY:
	{
		int r = pParse->onParseKey(fd, pBuf, unparselen, addr);				
		if (r == 0) {
			return pParse->onParseMsg(fd, pBuf, unparselen, addr);
		}
		return r;
	}
	case DATAERROR:
	case HEADNOCOMP:
	case DATANOCOMP:
	default:
		return -1;
	}
}


}