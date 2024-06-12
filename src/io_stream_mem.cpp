#include <string>
#include <assert.h>
#include <iostream>
#include "io_stream_mem.h"
#include "io_stream_defind.h"

namespace PLATFORM {

CBuffer::CBuffer(int size, int maxFrames)
: m_iReference(0)
{
	m_iMaxFrame = maxFrames;
	m_iSize = size;
	m_cBuffer = (char*)malloc(sizeof(STREAMBUFFERINFO) * MAX_CONSUMER + SHARE_MEM_HEAD_SIZE(m_iMaxFrame)+size);
	memset(m_cBuffer, 0x00, sizeof(STREAMBUFFERINFO) * MAX_CONSUMER + SHARE_MEM_HEAD_SIZE(m_iMaxFrame)+size);
}

CBuffer::~CBuffer()
{
	if (m_cBuffer) {
		free(m_cBuffer);
		m_cBuffer = NULL;
	}
}

char* CBuffer::GetBuffer()
{
	return m_cBuffer + sizeof(STREAMBUFFERINFO) * MAX_CONSUMER;
}

CBuffer* CBuffer::AddRefren()
{
	std::unique_lock<std::mutex> lk(m_mtxBuff);
	m_iReference ++;
	return this;
}

int CBuffer::DelRefren()
{
	std::unique_lock<std::mutex> lk(m_mtxBuff);
	--m_iReference;
	return m_iReference;
}

int CBuffer::GetMaxFrame()   
{
	return m_iMaxFrame;
}

int	CBuffer::GetSize() 
{
	return m_iSize;
}

void CBuffer::GetStreamInfo(STREAMBUFFERINFO& streamInfo)
{
	unsigned int min = 0xffffffff;
	int index = -1;
	STREAMBUFFERINFO* pStreamBuf = (STREAMBUFFERINFO*)m_cBuffer;
	ShmUsrInfo* pShmUsrInfo = (ShmUsrInfo*)(m_cBuffer + sizeof(STREAMBUFFERINFO)*MAX_CONSUMER);
	for (int i = 0; i < MAX_CONSUMER; ++i) {
		if (pShmUsrInfo->consumer[i].bUse) {
			if (pStreamBuf[i].u32bitrate < min && pStreamBuf[i].u32bitrate > 0) {
				min = pStreamBuf[i].u32bitrate;
				index = i;
			}
		}
	}

	if (index != -1) {
		memcpy(&streamInfo, &pStreamBuf[index], sizeof(streamInfo));
	}else{
		memset(&streamInfo, 0x00, sizeof(streamInfo));
	}
}

void CBuffer::SetStreamInfo(int index, STREAMBUFFERINFO streamInfo)
{
	STREAMBUFFERINFO* pStreamBuf = (STREAMBUFFERINFO*)m_cBuffer;
	if (index >= 0 && index < MAX_CONSUMER) {
		memcpy(&pStreamBuf[index], &streamInfo, sizeof(streamInfo));
	}
}

}