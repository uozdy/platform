#ifndef _WtMediaSdk_BYLOG_H__
#define _WtMediaSdk_BYLOG_H__

#ifdef  __cplusplus
extern "C"{
#endif
typedef enum
{
	WT_L_ERR,         /* ���ش��� 0*/
	WT_L_WARN,        /* һ�㾯�� 1*/
	WT_L_INFO,        /* ��ʾ��Ϣ(���д�ӡ) 2*/
	WT_L_NONE,        /* һ���ӡ(���Դ�ӡ) 3*/  
	WT_L_MAX,
}WT_LOG_LEVEL_E;



#define BYLOG_E(Fmt, ...)  LogPrintWithModName(__FILE__, __LINE__, WT_L_ERR, Fmt, ##__VA_ARGS__)
#define BYLOG_W(Fmt, ...)  LogPrintWithModName(__FILE__, __LINE__, WT_L_WARN, Fmt, ##__VA_ARGS__)
#define BYLOG_I(Fmt, ...)  LogPrintWithModName(__FILE__, __LINE__, WT_L_INFO, Fmt, ##__VA_ARGS__)
#define BYLOG_D(Fmt, ...)  LogPrintWithModName(__FILE__, __LINE__, WT_L_NONE, Fmt, ##__VA_ARGS__)

#define MAX_MSG_BUFF_LEN 2048
typedef struct 
{
	long mtype;
	int  eLogLevel;
	char data[MAX_MSG_BUFF_LEN];
} MsgLog;


void LogPrintWithModName(
							const char* pcFile, 
							int   iLine, 
							WT_LOG_LEVEL_E eLogLevel, 
							const char* pcFmt,
							...);
							
int GetLog(MsgLog* pLog);


#ifdef  __cplusplus
}
#endif
								
#endif
