#ifndef _CSTREAMBUFF_H__
#define _CSTREAMBUFF_H__

#include <stdio.h>
#include <string.h>
#include <mutex>
#include <map>

namespace PLATFORM {
#ifndef MAX_CONSUMER
#define MAX_CONSUMER		10
#endif

enum ShmAccessMode
{
	SHM_ACCESS_READ,
	SHM_ACCESS_WRITE_NONBLOCK,
	SHM_ACCESS_WRITE_BLOCK,
};

#ifndef DEF_STREAMBUFFERINFO
#define DEF_STREAMBUFFERINFO
typedef struct
{
	unsigned int   u32bitrate;
	unsigned int   reverse;
}STREAMBUFFERINFO;
#endif

class CStreamBuff
{
public:
	CStreamBuff(const char* name, const char* usr, int accessMode, int size, int maxFrames);
	virtual ~CStreamBuff();
	
public:
	int getFrameWithHead(char* pHead, int headLen, char* pFrameData, int frameLen, int *isKeyFrame, unsigned long long *pts, int *unreadCount);
	int putFrameWithHead(const char* head, int headlen, const char* pFrameData, int frameLen, int writeMode, int isKeyFrame);
	int putFrame(const char* pFrameData, int frameLen, int writeMode, int isKeyFrame);
	int getFrame(char* pFrameData, int frameLen, int *isKeyFrame);
	int getFrame(char* pFrameData, int frameLen, int *isKeyFrame, unsigned long long *pts);
	int getFrame(char* pFrameData, int frameLen, int *isKeyFrame, unsigned long long *pts, int *unreadCount);
	int GetLatestIFrame(int userIdx, int frameLen, unsigned int oldWritePos, bool bJumpEndFrame);
	
	bool GetStreamInfo(STREAMBUFFERINFO& streamInfo);
	bool SetStreamInfo(STREAMBUFFERINFO info);
	
	int GetUnreadCount();
	int GetSavedMemSize();
	int rPosUpdate();
	int clearBuff();
	
	static void Debug(int fd, void* usrdata, int argc, char*argv[]);
	
protected:
	bool isWriteable(int accessMode, int frameLen);
	bool adjustWritePtrBlockMode(int frameLen);  
	bool adjustRWPtrNoBlockMode(int frameLen);
	
private: 
	void*			m_pRealBuf;
	std::string		m_sName;
	std::string		mUsr;
	
	char*			mpShareMem;
	int				mShmSize;
	int				mMaxFrames;
	int				mAccessMode;
	int				mUsrIndex;
	
	static std::mutex	m_vMutex;
	static std::map<std::string, void*>  m_vBuffer; 
};

}
#endif 
