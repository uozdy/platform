#ifndef _CSTREAM_MEM_H__
#define _CSTREAM_MEM_H__
#include <stdio.h>
#include <string.h>
#include "io_stream_defind.h"

namespace PLATFORM {

class CBuffer
{
public:
	CBuffer(int size, int maxFrames);
	virtual ~CBuffer();
	
public:
	CBuffer* AddRefren();
	int DelRefren();
	
	char* GetBuffer();
	int GetMaxFrame();
	int	GetSize();
	
	void GetStreamInfo(STREAMBUFFERINFO& streamInfo);
	void SetStreamInfo(int index, STREAMBUFFERINFO streamInfo);
	
	std::mutex&	mutex() {return m_mtxBuff;}
	
private: 
	char*			m_cBuffer;
	int				m_iMaxFrame;
	int				m_iSize;
	int				m_iReference;
	
	std::mutex		m_mtxBuff;
};

}
#endif 
