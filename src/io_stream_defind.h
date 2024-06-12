#ifndef _BUFFERDEFIND_H__
#define _BUFFERDEFIND_H__

#include <stdio.h>
#include <stdlib.h>
#include <mutex>


namespace PLATFORM
{
	
#ifndef MAX_CONSUMER
#define MAX_CONSUMER		10
#endif

typedef enum{
	OFFSETFRAME_NONE = 0x0,
	OFFSETFRAME_IFRAME = 0x1,
	OFFSETFRAME_PFRAME = 0x2,
}OFFSETFRAME;


#ifndef DEF_STREAMBUFFERINFO
#define DEF_STREAMBUFFERINFO
typedef struct
{
	unsigned int   u32bitrate;
	unsigned int   reverse;
}STREAMBUFFERINFO;
#endif

typedef struct tagShmProductorInfo
{
	int					offset;		//下一次想写的帧头
	unsigned int		iFrameNO;	//下一次想写的帧序号
	unsigned int		wCount;		//成功写入的次数
}ShmProductorInfo;    //productor head

typedef struct tagShmConsumerInfo
{
	bool			bUse;			//读者是否启用
	OFFSETFRAME		offsetFrame;	//帧状态, OFFSETFRAME_NONE时,只能从I帧开始正常读数据，开始前读到非I帧丢弃
	int				offset;			//下一次想读的帧头
	unsigned int	iFrameNO;		//最后一次读取时的帧的帧序号
	unsigned int	rCount;			//读取帧成功的次数
}ShmConsumerInfo;  //Consumer head

typedef struct tagShmUsrInfo
{
	ShmProductorInfo productor;
	ShmConsumerInfo	consumer[MAX_CONSUMER];
}ShmUsrInfo;    //user head

typedef struct tagShmDataHead
{
	int			position;		//指向的数据在GetBuffer()后的偏移
	int			length;			//帧长
	int			isKeyFrame;		//是否时关键帧
	unsigned int			iFrameNO;		//帧序号
	unsigned long long		pts;			//时间戳
} ShmDataHead;  //FrameInfo head

//one streambuffer compoums with STREAMBUFFERINFO + ShmUsrInfo + ShmDataHead
#define SHARE_MEM_HEAD_SIZE(n) (sizeof(ShmUsrInfo) + n*sizeof(ShmDataHead))  
}

#endif
