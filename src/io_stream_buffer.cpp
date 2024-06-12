#include <string>
#include <assert.h>
#include <iostream>
#include "ByTime.h"
#include "io_stream_buffer.h"
#include "io_stream_defind.h"
#include "io_stream_mem.h"
#include "platform.h"
namespace PLATFORM
{
	
std::map<std::string, void*>  CStreamBuff::m_vBuffer;
std::mutex CStreamBuff::m_vMutex;

bool CStreamBuff::GetStreamInfo(STREAMBUFFERINFO& streamInfo)
{
	std::unique_lock<std::mutex> lk(((CBuffer*)m_pRealBuf)->mutex());
	((CBuffer*)m_pRealBuf)->GetStreamInfo(streamInfo);
	return true;
}

bool CStreamBuff::SetStreamInfo(STREAMBUFFERINFO info)
{
	std::unique_lock<std::mutex> lk(((CBuffer*)m_pRealBuf)->mutex());	
	((CBuffer*)m_pRealBuf)->SetStreamInfo(mUsrIndex, info);
	return true;
}

CStreamBuff::CStreamBuff(const char* name, const char* usr, int accessMode, int size, int maxFrames)
: m_sName(name)
, mpShareMem(NULL)
{
	std::unique_lock<std::mutex> lk(m_vMutex);
	
	if (m_vBuffer[name] == NULL) {
		m_vBuffer[name] = new CBuffer(size, maxFrames);
	}
	
	m_pRealBuf = ((CBuffer*)m_vBuffer[name])->AddRefren();
	mMaxFrames = ((CBuffer*)m_pRealBuf)->GetMaxFrame();
	mShmSize = ((CBuffer*)m_pRealBuf)->GetSize();
	mpShareMem = ((CBuffer*)m_pRealBuf)->GetBuffer();
	mUsrIndex = 0;
	mUsr = usr;
	
	mAccessMode = accessMode;
	ShmUsrInfo *pUsrInfo = (ShmUsrInfo* )mpShareMem;
	
	if( (accessMode == SHM_ACCESS_WRITE_NONBLOCK) ||
		(accessMode == SHM_ACCESS_WRITE_BLOCK) )  // write mode
	{
		pUsrInfo->productor.offset = 0;
		pUsrInfo->productor.wCount = 0;
		
		std::unique_lock<std::mutex> lk(((CBuffer*)m_pRealBuf)->mutex());
		for( int i = 0; i < MAX_CONSUMER; i++ ) {
			if( !pUsrInfo->consumer[i].bUse ) {
				pUsrInfo->consumer[i].offset = 0;
				pUsrInfo->consumer[i].rCount = 0;
				pUsrInfo->consumer[i].offsetFrame = OFFSETFRAME_NONE;
			}
		}
		//BYLOG_W("[STREAM BUFFER] CreateStream(%s) usr:%s Writer mpShareMem:%p", name, mUsr.c_str(), mpShareMem);
	}
	else if( accessMode == SHM_ACCESS_READ )
	{
		bool isUsrExist = false;
		for ( int i=0; i < MAX_CONSUMER; i++) {
			if ( !pUsrInfo->consumer[i].bUse ) {
				isUsrExist = true;
				mUsrIndex = i;
				
				pUsrInfo->consumer[i].offset = pUsrInfo->productor.offset;
				pUsrInfo->consumer[i].bUse = true;
				pUsrInfo->consumer[i].rCount = pUsrInfo->productor.wCount;
                pUsrInfo->consumer[i].offsetFrame = OFFSETFRAME_NONE;
                pUsrInfo->consumer[i].iFrameNO = pUsrInfo->productor.iFrameNO;
				break;
			}
		}
		
		if( !isUsrExist ) {
			assert(0);
		}

		//BYLOG_W("[STREAM BUFFER] CreateStream(%s) usr:%s Reader mpShareMem:%p", name, mUsr.c_str(), mpShareMem);
	}
}

CStreamBuff::~CStreamBuff()
{
	try{
		std::unique_lock<std::mutex> lk(m_vMutex);
		if (m_vBuffer[m_sName.c_str()]) {
			if ( ((CBuffer*)m_vBuffer[m_sName.c_str()])->DelRefren() == 0) {
				delete ((CBuffer*)m_vBuffer[m_sName.c_str()]);
				m_vBuffer.erase(m_sName.c_str());
			}
			else{
				if( (mAccessMode == SHM_ACCESS_WRITE_NONBLOCK) ||
					(mAccessMode == SHM_ACCESS_WRITE_BLOCK) ) {
					//BYLOG_W("[STREAM BUFFER] ReleaseStream[%s] Writer, user:%s", m_sName.c_str(), mUsr.c_str());
				}
				else {
					if(MAX_CONSUMER>mUsrIndex && 0<=mUsrIndex) {
						ShmUsrInfo *pUsrInfo = (ShmUsrInfo* )mpShareMem;
						pUsrInfo->consumer[mUsrIndex].bUse = false;
						pUsrInfo->consumer[mUsrIndex].offset = 0;
						pUsrInfo->consumer[mUsrIndex].rCount = 0;

						//BYLOG_W("[STREAM BUFFER] ReleaseStream[%s] Reader:%d, user:%s", m_sName.c_str(), mUsrIndex, mUsr.c_str());
					}
				}

			}
		}
	}catch(...){}
}

int CStreamBuff::putFrameWithHead(const char* phead, int headlen, const char* pFrameData, int frameLen, int writeMode, int isKeyFrame)
{
	int writeBytes = 0;
	if( (pFrameData == NULL) || (frameLen <= 0) ) {
		return 0;
	}

	if ( NULL == mpShareMem ) {
		return -1; 
	}

	std::unique_lock<std::mutex> lk(((CBuffer*)m_pRealBuf)->mutex());
	
	int head = 0, position = 0;
	ShmUsrInfo* pUsrInfo = (ShmUsrInfo* )mpShareMem;
	ShmDataHead* pDataHead = (ShmDataHead* )( mpShareMem + sizeof(ShmUsrInfo) );
	head = pUsrInfo->productor.offset % mMaxFrames;
	position = pDataHead[head].position;
	
	int totalWriteLen = frameLen + headlen;
	if( (isWriteable(writeMode, totalWriteLen)) && (totalWriteLen < mShmSize/2) ) {
		position = pDataHead[head].position;
		memcpy( mpShareMem+position, phead, headlen );
		memcpy( mpShareMem+position+headlen, pFrameData, frameLen );
		pDataHead[head].length = totalWriteLen;
		pDataHead[head].isKeyFrame = isKeyFrame;
		pDataHead[head].iFrameNO = ++pUsrInfo->productor.iFrameNO;
		pDataHead[head].pts = CTime::GetUTCTime_Ms();
		pDataHead[ (head+1) % mMaxFrames].position = position + totalWriteLen;
		pUsrInfo->productor.offset = (pUsrInfo->productor.offset + 1) % mMaxFrames;
		writeBytes = totalWriteLen;
		pUsrInfo->productor.wCount++;
	}else{
		//BYLOG_W("isWriteable(writeMode, frameLen):%d  (frameLen < mShmSize/2)%d  frameLen%d\n",isWriteable(writeMode, frameLen),(frameLen < mShmSize/2),frameLen);
	}
	return writeBytes;
}
	
int CStreamBuff::putFrame(const char* pFrameData, int frameLen, int writeMode, int isKeyFrame)
{
	int writeBytes = 0;

	if( (pFrameData == NULL) || (frameLen <= 0) ) {
		return 0;
	}

	if ( NULL == mpShareMem ) {
		return -1; 
	}

	std::unique_lock<std::mutex> lk(((CBuffer*)m_pRealBuf)->mutex());
	
	int head = 0, position = 0;
	ShmUsrInfo *pUsrInfo = (ShmUsrInfo* )mpShareMem;
	ShmDataHead* pDataHead = (ShmDataHead* )( mpShareMem + sizeof(ShmUsrInfo) );
	head = pUsrInfo->productor.offset % mMaxFrames;
	position = pDataHead[head].position;
	
	if( (isWriteable(writeMode, frameLen)) && (frameLen < mShmSize/2) ) {
		position = pDataHead[head].position;
		memcpy( mpShareMem+position, pFrameData, frameLen );
		pDataHead[head].length = frameLen;
		pDataHead[head].isKeyFrame = isKeyFrame;
		pDataHead[head].iFrameNO = ++pUsrInfo->productor.iFrameNO;
		pDataHead[head].pts = CTime::GetUTCTime_Ms();
		pDataHead[ (head+1) % mMaxFrames].position = position + frameLen;
		pUsrInfo->productor.offset = (pUsrInfo->productor.offset + 1) % mMaxFrames;
		writeBytes = frameLen;
		pUsrInfo->productor.wCount++;
	}else {
		//BYLOG_W("isWriteable(writeMode, frameLen):%d  (frameLen < mShmSize/2)%d  frameLen%d\n",isWriteable(writeMode, frameLen),(frameLen < mShmSize/2),frameLen);
	}
	return writeBytes;
}

int CStreamBuff::getFrame(char* pFrameData, int frameLen, int *isKeyFrame)
{
	int count = 0;
	unsigned long long pts = 0;
	return getFrame(pFrameData, frameLen, isKeyFrame, &pts, &count);
}

int CStreamBuff::getFrame(char* pFrameData, int frameLen, int *isKeyFrame, unsigned long long *pts)
{
	int count = 0;
	return getFrame(pFrameData, frameLen, isKeyFrame, pts, &count);
}

int CStreamBuff::getFrame(char* pFrameData, int frameLen, int *isKeyFrame, unsigned long long *pts, int *unreadCount)
{
	int ret = 0;
	if ( NULL == mpShareMem ) {
		return -1; 
	}
	
	std::unique_lock<std::mutex> lk(((CBuffer*)m_pRealBuf)->mutex());
	
	int head = 0, tail = 0, position = 0, len = 0;
	ShmUsrInfo *pUsrInfo = (ShmUsrInfo* )mpShareMem;
	ShmDataHead* pDataHead = (ShmDataHead* )( mpShareMem + sizeof(ShmUsrInfo) );
	head = pUsrInfo->productor.offset % mMaxFrames;
	tail = pUsrInfo->consumer[mUsrIndex].offset % mMaxFrames; 
	OFFSETFRAME& offsetFrame = pUsrInfo->consumer[mUsrIndex].offsetFrame;
	
	if( head != tail )
	{
		
		len = pDataHead[tail].length;
		position = pDataHead[tail].position;
		*isKeyFrame = pDataHead[tail].isKeyFrame;
		
		/*if (mUsr[0] == 'A' && tail != 0) {
			printf("[%s:%p] head:%d, tail:%d isKey:%d len:%d \n", mUsr.c_str(), this, head, tail, *isKeyFrame, len);
		}*/
		
		if (pts != NULL){
			*pts = pDataHead[tail].pts;
		}
		if (frameLen >= len) {
			if (*isKeyFrame || offsetFrame == OFFSETFRAME_IFRAME) {
				offsetFrame = OFFSETFRAME_IFRAME;
			} else {
				pUsrInfo->consumer[mUsrIndex].iFrameNO = pDataHead[tail].iFrameNO;
				pUsrInfo->consumer[mUsrIndex].offset = (tail + 1 ) % mMaxFrames;
				pUsrInfo->consumer[mUsrIndex].rCount++;
				return 0;
			}

			memcpy( pFrameData, mpShareMem + position, len );
			ret = len;
			
			pUsrInfo->consumer[mUsrIndex].iFrameNO = pDataHead[tail].iFrameNO;
			pUsrInfo->consumer[mUsrIndex].offset = (tail + 1 ) % mMaxFrames;
			pUsrInfo->consumer[mUsrIndex].rCount++;
		} else {
			ret = 0;
		}	

		int wcnt = pUsrInfo->productor.iFrameNO;
		int rcnt = pUsrInfo->consumer[mUsrIndex].iFrameNO;
		*unreadCount = wcnt - rcnt;
	}

	return ret;
}

int CStreamBuff::getFrameWithHead(char* pHead, int headLen, char* pFrameData, int frameLen, int *isKeyFrame, unsigned long long *pts, int *unreadCount)
{
	int ret = 0;
	if ( NULL == mpShareMem ) {
		return -1; 
	}
	
	std::unique_lock<std::mutex> lk(((CBuffer*)m_pRealBuf)->mutex());
	
	int head = 0, tail = 0, position = 0, len = 0;
	ShmUsrInfo *pUsrInfo = (ShmUsrInfo* )mpShareMem;
	ShmDataHead* pDataHead = (ShmDataHead* )( mpShareMem + sizeof(ShmUsrInfo) );
	head = pUsrInfo->productor.offset % mMaxFrames;
	tail = pUsrInfo->consumer[mUsrIndex].offset % mMaxFrames; 
	OFFSETFRAME& offsetFrame = pUsrInfo->consumer[mUsrIndex].offsetFrame;
	if( head != tail )
	{
		len = pDataHead[tail].length;
		position = pDataHead[tail].position;
		*isKeyFrame = pDataHead[tail].isKeyFrame;
		if (pts != NULL){
			*pts = pDataHead[tail].pts;
		}
		if ((frameLen+headLen) >= len) {
			if (*isKeyFrame || offsetFrame == OFFSETFRAME_IFRAME) {
				offsetFrame = OFFSETFRAME_IFRAME;
			} else {
				pUsrInfo->consumer[mUsrIndex].iFrameNO = pDataHead[tail].iFrameNO;
				pUsrInfo->consumer[mUsrIndex].offset = (tail + 1 ) % mMaxFrames;
				pUsrInfo->consumer[mUsrIndex].rCount++;
				return 0;
			}
			
			ret = len - headLen;
			memcpy( pHead, mpShareMem + position, headLen );	
			memcpy( pFrameData, mpShareMem + position + headLen, ret );			
			
			pUsrInfo->consumer[mUsrIndex].iFrameNO = pDataHead[tail].iFrameNO;
			pUsrInfo->consumer[mUsrIndex].offset = (tail + 1 ) % mMaxFrames;
			pUsrInfo->consumer[mUsrIndex].rCount++;
		} else {
			ret = 0;
		}	

		int wcnt = pUsrInfo->productor.iFrameNO;
		int rcnt = pUsrInfo->consumer[mUsrIndex].iFrameNO;
		*unreadCount = wcnt - rcnt;
	}

	return ret;
}

int CStreamBuff::rPosUpdate()
{
	if ( NULL == mpShareMem ) {
		//BYLOG_E("mpShareMem is null");
		return -1; 
	}
	
	//printf("[%s] rPosUpdate()", mUsr.c_str());
	if ( mAccessMode == SHM_ACCESS_READ ) {
		std::unique_lock<std::mutex> lk(((CBuffer*)m_pRealBuf)->mutex());
		ShmUsrInfo *pUsrInfo = (ShmUsrInfo* )mpShareMem;
		pUsrInfo->consumer[mUsrIndex].offset = pUsrInfo->productor.offset;
		pUsrInfo->consumer[mUsrIndex].offsetFrame = OFFSETFRAME_NONE;
		pUsrInfo->consumer[mUsrIndex].rCount = pUsrInfo->productor.wCount;
		pUsrInfo->consumer[mUsrIndex].iFrameNO = pUsrInfo->productor.iFrameNO;
	}

	return 0;
}

bool CStreamBuff::isWriteable(int accessMode, int frameLen)
{
	if( accessMode == SHM_ACCESS_WRITE_BLOCK ) {
		return adjustWritePtrBlockMode( frameLen );
	}else {
		return adjustRWPtrNoBlockMode( frameLen );
	}
}

bool CStreamBuff::adjustWritePtrBlockMode(int frameLen)
{
	int i;
	int head, tail;
	int writePosition;
	int readPosition;

	ShmUsrInfo *pUsrInfo = (ShmUsrInfo* )mpShareMem;
	ShmDataHead* pDataHead = (ShmDataHead* )( mpShareMem + sizeof(ShmUsrInfo) );

	head = pUsrInfo->productor.offset % mMaxFrames;  //生产者当前想写的帧头
	writePosition = pDataHead[head].position; //生产者当前想写的内存起始地址

	for( i = 0; i < MAX_CONSUMER; i++ ) {
		if( !pUsrInfo->consumer[i].bUse ) {
			continue;
		}
		
		tail = pUsrInfo->consumer[i].offset % mMaxFrames;
		if( head == tail ) {
			continue;
		}
		
		if( (head + 1 ) % mMaxFrames == tail ) {
			return false;
		}
		readPosition = pDataHead[tail].position;
		if( (writePosition <= readPosition) && (writePosition + frameLen >= readPosition) ) {
			return false;
		}
		
		if( ( (writePosition + frameLen ) >= mShmSize ) ) {
			if( (SHARE_MEM_HEAD_SIZE(mMaxFrames) + frameLen) > (unsigned int)readPosition ) {
				return false;
			}
		}
	}

	if( ( (writePosition + frameLen) >= mShmSize ) ||
		( (unsigned int)writePosition < SHARE_MEM_HEAD_SIZE(mMaxFrames) ) ) {
		pDataHead[head].position = SHARE_MEM_HEAD_SIZE(mMaxFrames);
	}

	return true;
}

bool CStreamBuff::adjustRWPtrNoBlockMode(int frameLen)
{
	int i;
	int head, tail, iFrame;
	int writePosition = 0;
	int writePosition_old = 0;
	
	ShmUsrInfo* pUsrInfo = (ShmUsrInfo* )mpShareMem;
	ShmDataHead* pDataHead = (ShmDataHead* )( mpShareMem + sizeof(ShmUsrInfo) );
	head = pUsrInfo->productor.offset % mMaxFrames;
	writePosition = pDataHead[head].position;
	
	//
	if ( ((writePosition + frameLen) >= mShmSize) ||   
		((unsigned int)writePosition < SHARE_MEM_HEAD_SIZE(mMaxFrames)) )
	{
		writePosition_old = writePosition;
		writePosition = SHARE_MEM_HEAD_SIZE(mMaxFrames);
		pDataHead[head].position = writePosition;
    }

	for( i = 0; i < MAX_CONSUMER; i++ )
	{
		if( !pUsrInfo->consumer[i].bUse ) {
			continue;
		}

		tail = pUsrInfo->consumer[i].offset % mMaxFrames;
		
		/*
		    ______________________________________________________________________
		    |  mMaxFrames个帧头     |           帧缓存                            |
			——————————————————————————————————————————————————————————————————————
			head == tail时,从帧头上识别出读者写者位置一致。
			head != tail时,需要判断写者的position+frameLen是否覆盖到了读者的position
			1)head == tail, consumer[i].iFrameNo < productor.iFrameNo-1 写者赶上了读者(读者赶上写者时,帧序号偏差不会超过1)
			2)head != tail
			  1. writePosition_old != 0 非第一次写且从帧缓存最开始写
				,writePosition_old <= pDataHead[tail].position 写着在读者后面
				,需要推动读者重新找帧(如果在跳过的这段空间里有I帧？bJumpEndFrame跳过这段空间)
			  2. 写着的写位置覆盖到读者，推动读者重新找帧
			  3. 读者写者的偏移一致，且帧序号差值大于1, 写者赶上了读者。推动读者重新找帧
		*/
		if ( ((head == tail) && (pUsrInfo->consumer[i].iFrameNO < (pUsrInfo->productor.iFrameNO-1) && writePosition_old != 0))
			|| ((head != tail) 
			   && (
			          (writePosition_old != 0 && writePosition_old <= pDataHead[tail].position )
			       || ((writePosition < pDataHead[tail].position) && (writePosition + frameLen >= pDataHead[tail].position))   
			       || (writePosition == pDataHead[tail].position && (pUsrInfo->consumer[i].iFrameNO < (pUsrInfo->productor.iFrameNO-1)))
			      )
			   )
			)
		{
			bool bJumpEndFrame = (bool)(head != tail && writePosition_old != 0 && writePosition_old <= pDataHead[tail].position);
			iFrame = GetLatestIFrame( i, frameLen, writePosition_old, bJumpEndFrame);

			pUsrInfo->consumer[i].offset = iFrame;
			//BYLOG_W(" [STREAM BUFFER:%s] %s to find IFrame：%d %d %d", m_sName.c_str(), mUsr.c_str(), pUsrInfo->consumer[i].offsetFrame, i, pUsrInfo->productor.iFrameNO);
		}
	}

	return true;
}

int CStreamBuff::GetLatestIFrame(int userIdx, int frameLen, unsigned int oldWritePos, bool bJumpEndFrame)
{
	int i,head, tail, iFrame;
	unsigned int writePosition;
	unsigned int readPosition;

	ShmUsrInfo *pUsrInfo = (ShmUsrInfo* )mpShareMem;
	ShmDataHead* pDataHead = (ShmDataHead* )( mpShareMem + sizeof(ShmUsrInfo) );
    pUsrInfo->consumer[userIdx].offsetFrame = OFFSETFRAME_NONE;
	tail = pUsrInfo->consumer[userIdx].offset + 1;  
	head = pUsrInfo->productor.offset;
	writePosition = pDataHead[head].position;
	iFrame = head; 

	bool bFind = false;
	for( i = 0; i < mMaxFrames-1; i++ ) {
		if( tail  == head ) {
			pUsrInfo->consumer[userIdx].offsetFrame = OFFSETFRAME_NONE;
			pUsrInfo->consumer[userIdx].rCount += i;
			pUsrInfo->consumer[userIdx].iFrameNO = pUsrInfo->productor.iFrameNO;
			return head;
		}
		
		if( pDataHead[tail].isKeyFrame == 1 ) {
			readPosition = pDataHead[tail].position;
			if (bJumpEndFrame && oldWritePos < readPosition) {
				tail = (tail + 1) % mMaxFrames;
				continue;
			}
			
			if (writePosition == readPosition) {
				pUsrInfo->consumer[userIdx].offsetFrame = OFFSETFRAME_NONE;
				pUsrInfo->consumer[userIdx].rCount += i;
				pUsrInfo->consumer[userIdx].iFrameNO = pUsrInfo->productor.iFrameNO;
				return head;
			}
			
			if( (writePosition < readPosition) &&
				(writePosition + frameLen >= readPosition) ) {
				tail = (tail + 1) % mMaxFrames;
				continue;
			}

			bFind = true;
			iFrame = tail;
			pUsrInfo->consumer[userIdx].offsetFrame = OFFSETFRAME_IFRAME;
			pUsrInfo->consumer[userIdx].rCount += i;
			pUsrInfo->consumer[userIdx].iFrameNO = pDataHead[tail].iFrameNO;
			break;
		}else{
            readPosition = pDataHead[tail].position;
            if (writePosition == readPosition) {
                pUsrInfo->consumer[userIdx].offsetFrame = OFFSETFRAME_NONE;
                pUsrInfo->consumer[userIdx].rCount += i;
                pUsrInfo->consumer[userIdx].iFrameNO = pUsrInfo->productor.iFrameNO;
                return head;
            }
		}
		tail = (tail + 1) % mMaxFrames;
	}

	if (!bFind)
		pUsrInfo->consumer[userIdx].rCount += mMaxFrames;

	return iFrame;
}

int CStreamBuff::clearBuff()
{
	if ( NULL == mpShareMem ) {
		return -1; 
	}

	//printf("[%s] clearBuff()", mUsr.c_str());
	//if ( mAccessMode == SHM_ACCESS_WRITE_NONBLOCK || mAccessMode == SHM_ACCESS_WRITE_BLOCK )
	{
		std::unique_lock<std::mutex> lk(((CBuffer*)m_pRealBuf)->mutex());
		ShmUsrInfo *pUsrInfo = (ShmUsrInfo* )mpShareMem;
		for ( int i = 0; i < MAX_CONSUMER; i++ )
		{
			if (pUsrInfo->consumer[i].bUse)
			{
				pUsrInfo->consumer[i].offset = pUsrInfo->productor.offset;
				pUsrInfo->consumer[i].offsetFrame = OFFSETFRAME_NONE;
				pUsrInfo->consumer[i].iFrameNO = pUsrInfo->productor.iFrameNO;
				pUsrInfo->consumer[i].rCount = pUsrInfo->productor.wCount;
			}
		}
	}

	return 0;
}

int CStreamBuff::GetUnreadCount()
{
	if ( NULL == mpShareMem ) {
		return -1; 
	}
	int wcnt = 0, rcnt = 0;
	ShmUsrInfo *pUsrInfo = (ShmUsrInfo* )mpShareMem;

	wcnt = pUsrInfo->productor.iFrameNO;
	rcnt = pUsrInfo->consumer[mUsrIndex].iFrameNO;
	return (wcnt- rcnt);
}

int CStreamBuff::GetSavedMemSize()
{
	if ( NULL == mpShareMem ) {
		//BYLOG_E("mpShareMem is null\n");
		return -1; 
	}
	
	std::unique_lock<std::mutex> lk(((CBuffer*)m_pRealBuf)->mutex());
	
	ShmUsrInfo *pUsrInfo = (ShmUsrInfo* )mpShareMem;
	ShmDataHead* pDataHead = (ShmDataHead* )( mpShareMem + sizeof(ShmUsrInfo) );
	int head = pUsrInfo->productor.offset % mMaxFrames;
	int writePosition = pDataHead[head].position;
	int tail = pUsrInfo->consumer[mUsrIndex].offset % mMaxFrames;
	int ReadPosition = pDataHead[tail].position;

	if ((pUsrInfo->productor.iFrameNO > pUsrInfo->consumer[mUsrIndex].iFrameNO))
	{
		if (head >= tail)
		{
			if(writePosition > ReadPosition)
			{
				return (writePosition - ReadPosition);
			}
			else if (writePosition <= ReadPosition)
			{
				return (mShmSize + writePosition - ReadPosition);
			}
		}
		if (head < tail)
		{
			if (writePosition <= ReadPosition)
			{
				return (mShmSize + writePosition - ReadPosition);
			}	
		}	
	}
	return 0;
}

void CStreamBuff::Debug(int fd, void* usrdata, int argc, char*argv[])
{
	std::unique_lock<std::mutex> lk(m_vMutex);
	std::map<std::string, void*>::iterator it = m_vBuffer.begin();  
	for (; it != m_vBuffer.end(); ++it) {
		CBuffer* pBuffer = ((CBuffer*)it->second);
		char* pShareMem = pBuffer->GetBuffer();
		ShmUsrInfo *pUsrInfo = (ShmUsrInfo* )pShareMem;
		
		std::unique_lock<std::mutex> lk(pBuffer->mutex());
		for ( int i=0; i < MAX_CONSUMER; i++) {

			if (pUsrInfo->consumer[i].bUse){
							
				int wcnt = pUsrInfo->productor.iFrameNO;
				int rcnt = pUsrInfo->consumer[i].iFrameNO;
				int readCount = wcnt - rcnt;
				
				char buf[1024] = {0};
				sprintf(buf, "name:%s index:%d unreadCount:%d\n", it->first.c_str(), i, readCount);
				write(fd, buf, strlen(buf));
			}
		}
	}
}

}