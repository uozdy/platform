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

/*��־��ɫ����*/
#define ANSI_GRAY          "\033[01;30m"       /* ��ɫ */
#define ANSI_BLINK_RED     "\033[00;31m"       /* ��ɫ */
#define ANSI_RED           "\033[01;31m"       /* �����ɫ */
#define ANSI_BLINK_GREEN   "\033[00;32m"       /* ��ɫ */
#define ANSI_GREEN         "\033[01;32m"       /* ��ɫ */
#define ANSI_BLINK_YELLOW  "\033[00;33m"       /* ��ɫ */
#define ANSI_YELLOW        "\033[01;33m"       /* ��ɫ */
#define ANSI_BLINK_BLUE    "\033[00;34m"       /* ��ɫ */
#define ANSI_BLUE          "\033[01;34m"       /* ��ɫ */
#define ANSI_MAGENTA       "\033[01;35m"       /* XXɫ */
#define ANSI_CYAN          "\033[01;36m"       /* XXɫ */
#define ANSI_WHITE         "\033[01;37m"       /* ��ɫ */
#define ANSI_NONE          "\033[00m"          /* ��ɫ */

namespace PLATFORM {
char g_szModuleName[64] = {0};
static int g_iPid = 0;
static int g_iLogFd = 0;

/*��־��ʾ����*/
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

    /*��ȡʱ��*/
    stTime = time(NULL);
    if (NULL == (pstLocalTime = localtime(&stTime)))
    {
        return;
    }
    
    /*��ȡ�ļ���*/
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

    /*��ȡ��־ʱ����ļ���*/
    LogGetPrintTimeAndFileName(&pstLocalTime, &pcFileName, pcFile);

	PLATFORM::g_szModuleName[10] = 0;
    /*��ʽ����־ͷ*/
	iLen = snprintf(logBuffer.data, sizeof(logBuffer.data) - 1,
                    "%s[%10s:%d] %04d-%02d-%02d "		/*(��ɫ)+ģ����:���̺� ����*/
                    "%02d:%02d:%02d "					/*ʱ��*/
                    "[%20s:%4d]"						/*�ļ���+��*/ 
                    " %6s",            					/*��־����*/
					PLATFORM::g_stLogPrint[eLogLevel].pcLogLevelColor, PLATFORM::g_szModuleName, PLATFORM::g_iPid, 
                    pstLocalTime->tm_year + 1900, pstLocalTime->tm_mon + 1, pstLocalTime->tm_mday, 
                    pstLocalTime->tm_hour, pstLocalTime->tm_min, pstLocalTime->tm_sec,  
                    pcFileName, iLine,
                    PLATFORM::g_stLogPrint[eLogLevel].pcLogLevelStr);

    /*��ʽ������*/
    va_start(ap, pcFmt);
    iLen += vsnprintf(logBuffer.data + iLen, MAX_MSG_BUFF_LEN - iLen - 1,  pcFmt, ap);
    va_end(ap);

#if 0
    /*������ն�*/
    fprintf(stderr, ANSI_NONE);
    fprintf(stderr, "%d:%s", iLen, logBuffer.data);
    fprintf(stderr, ANSI_NONE);
    fprintf(stderr,"\n");
    fflush(stderr);
#endif
	msgsnd( PLATFORM::g_iLogFd, &logBuffer, iLen + sizeof(int)+1, IPC_NOWAIT );
}
#endif

