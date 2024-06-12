#ifndef __WTMEDIASDK_SRC_LOG__
#define __WTMEDIASDK_SRC_LOG__
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "platform.h"
#include "ByLog.h"
#include "ByTime.h"

#define LOGID		20

#define LOG_LEVEL_NONE_STR  "[NONE]: "
#define LOG_LEVEL_INFO_STR  "[INFO]: "
#define LOG_LEVEL_WARN_STR  "[WARN]: "
#define LOG_LEVEL_ERR_STR   "[ERR ]: "

/*日志颜色定义*/
#define ANSI_GRAY          "\033[01;30m"       /* 灰色 */
#define ANSI_BLINK_RED     "\033[00;31m"       /* 红色 */
#define ANSI_RED           "\033[01;31m"       /* 粗体红色 */
#define ANSI_BLINK_GREEN   "\033[00;32m"       /* 绿色 */
#define ANSI_GREEN         "\033[01;32m"       /* 绿色 */
#define ANSI_BLINK_YELLOW  "\033[00;33m"       /* 黄色 */
#define ANSI_YELLOW        "\033[01;33m"       /* 黄色 */
#define ANSI_BLINK_BLUE    "\033[00;34m"       /* 蓝色 */
#define ANSI_BLUE          "\033[01;34m"       /* 蓝色 */
#define ANSI_MAGENTA       "\033[01;35m"       /* XX色 */
#define ANSI_CYAN          "\033[01;36m"       /* XX色 */
#define ANSI_WHITE         "\033[01;37m"       /* 白色 */
#define ANSI_NONE          "\033[00m"          /* 无色 */

namespace PLATFORM {
char g_szModuleName[64] = {0};
static int g_iPid = 0;
static int g_iLogFd = 0;

/*日志显示配置*/
typedef struct 
{
	const char *pcLogLevelStr;
	const char *pcLogLevelColor;
}WT_LOG_PRINT_ST;

class GlobelLog{
public:
GlobelLog() {
    g_iLogFd = msgget( 0x1, IPC_CREAT | 0666 );
	g_iPid = GetCurrentProcessId();
}

~GlobelLog(){
	
}

};


static GlobelLog sLogInit;

static WT_LOG_PRINT_ST g_stLogPrint[WT_L_MAX] = {
    {LOG_LEVEL_ERR_STR, ANSI_RED},
    {LOG_LEVEL_WARN_STR, ANSI_YELLOW},
    {LOG_LEVEL_INFO_STR, ANSI_GREEN},
    {LOG_LEVEL_NONE_STR, ANSI_NONE}
};

}

static void LogGetPrintTimeAndFileName
(
    struct tm **ppstLocalTime, 
    char **ppcTmpFile, 
    const char *pcFile)
{
    const char *pcTmpFile = NULL;

    time_t stTime;
    struct tm *pstLocalTime = NULL;

    /*获取时间*/
    stTime = time(NULL);
    if (NULL == (pstLocalTime = localtime(&stTime)))
    {
        return;
    }
    
    /*获取文件名*/
    pcTmpFile = strrchr(pcFile, '/');
    if ( NULL == pcTmpFile )
    {
        pcTmpFile = (char *)pcFile;
    }
    else
    {
        pcTmpFile++;
    }

    *ppstLocalTime = pstLocalTime;
    *ppcTmpFile = (char *)pcTmpFile;

    return;
}

extern "C" int GetLog(MsgLog* pLog)
{
	int len;
	while ( (len=msgrcv( PLATFORM::g_iLogFd, pLog, sizeof(MsgLog)-sizeof(long), LOGID, IPC_NOWAIT))  <= 0 ) {
		Sleep(10);
	}
	return len - sizeof(int);
}

extern "C" void LogPrintWithModName(
    const char* pcFile, 
    int   iLine, 
    WT_LOG_LEVEL_E eLogLevel, 
    const char* pcFmt,
    ...)
{
	MsgLog logBuffer;

    int iLen = 0;
    char *pcFileName = NULL;
    struct tm *pstLocalTime = NULL;
    va_list ap;
	
	logBuffer.mtype = LOGID;
	logBuffer.eLogLevel = eLogLevel;

    /*获取日志时间和文件名*/
    LogGetPrintTimeAndFileName(&pstLocalTime, &pcFileName, pcFile);

	PLATFORM::g_szModuleName[10] = 0;
    /*格式化日志头*/
	iLen = snprintf(logBuffer.data, sizeof(logBuffer.data) - 1,
                    "%s[%10s:%d] %04d-%02d-%02d "		/*(颜色)+模块名:进程号 日期*/
                    "%02d:%02d:%02d "					/*时间*/
                    "[%20s:%4d]"						/*文件名+行*/ 
                    " %6s",            					/*日志级别*/
					PLATFORM::g_stLogPrint[eLogLevel].pcLogLevelColor, PLATFORM::g_szModuleName, PLATFORM::g_iPid, 
                    pstLocalTime->tm_year + 1900, pstLocalTime->tm_mon + 1, pstLocalTime->tm_mday, 
                    pstLocalTime->tm_hour, pstLocalTime->tm_min, pstLocalTime->tm_sec,  
                    pcFileName, iLine,
                    PLATFORM::g_stLogPrint[eLogLevel].pcLogLevelStr);

    /*格式化参数*/
    va_start(ap, pcFmt);
    iLen += vsnprintf(logBuffer.data + iLen, MAX_MSG_BUFF_LEN - iLen - 1,  pcFmt, ap);
    va_end(ap);

#if 0
    /*输出到终端*/
    fprintf(stderr, ANSI_NONE);
    fprintf(stderr, "%d:%s", iLen, logBuffer.data);
    fprintf(stderr, ANSI_NONE);
    fprintf(stderr,"\n");
    fflush(stderr);
#endif
	msgsnd( PLATFORM::g_iLogFd, &logBuffer, iLen + sizeof(int)+1, IPC_NOWAIT );
}
#endif

