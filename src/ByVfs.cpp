#include "ByVfs.h"
#include "net_tcp_loop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <mutex>
#include <thread>
#include "io_socket_ext.h"
#include "platform.h"

namespace PLATFORM
{

	typedef struct{
		PrintVfs	fun;
		void*		usrdata;
	}BYVFS;
	
	class CByVfs: public IParse {
	public:
		static CByVfs* GetInstance();
		static int Init(const char* moduleName, const char* pipeName, short port);
		static void Register(const char* name, PrintVfs fun, void* usrdata);
		void waitMsg(int ms);
		
	private:
		CByVfs();
		~CByVfs();
        void SplitToStrs(char* buf, int maxBufLen, int& argc, char** argv, int argNum);
        void SendHelp(int fd);
		
	protected:
		virtual int onParseMsg(int fd, char* pData, int len, struct sockaddr* addr);
		virtual char* GetBufInfo(int* unparselen, int* maxlen);
		virtual void SetUnParseLen(int unparselen);
		
		static void onEvent(int fd, int msg, void* wParam, void* lParam);
	private:
		CNetTcpLoop 	m_netloop;
		std::map<std::string, BYVFS> 	mFuncMap;
		
		int 			m_fdStdOut;
		int 			m_fdStdErr;
		int				m_fdLog;
		
		char*			m_pRecvBuffer;
		int				m_iUnParselen;
		int				m_iBufferLen;

		static char 		CLIHEAD[30];
		static std::mutex	s_lock;
		static CByVfs* 		s_pInstance;
	};

	char CByVfs::CLIHEAD[30] = {0};
	std::mutex CByVfs::s_lock;
	CByVfs* CByVfs::s_pInstance = NULL;
	CByVfs::CByVfs()
		: m_netloop()
		, m_fdStdOut(0)
		, m_fdStdErr(0)
		, m_fdLog(0)
	{
		m_iBufferLen = 1024*20;
		m_pRecvBuffer = (char*)malloc(m_iBufferLen);
		m_iUnParselen = 0;
	}

	CByVfs::~CByVfs()
	{
		free(m_pRecvBuffer);
	}
	
	CByVfs* CByVfs::GetInstance()
	{
		if (!s_pInstance) {
			s_pInstance = new CByVfs();
		}
		return s_pInstance;
	}
	
	void CByVfs::onEvent(int fd, int msg, void* wParam, void* lParam)
	{
		switch (msg) {
			case ENETMSG_ONACCEPT: {
				int flag = 1;
				setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
				s_pInstance->m_netloop.AddClt(fd, CByVfs::onEvent, [](int socket, char* pRecvBuf, int iMaxSize, struct sockaddr* addr)->int {
					return recv(socket, pRecvBuf, iMaxSize, 0);
				}, s_pInstance, NULL);
				write(fd, CLIHEAD+1, strlen(CLIHEAD)-1);
				break;
			}
			case ENETMSG_ONRELEASE:				
				if (s_pInstance->m_fdLog == fd) {
					dup2(s_pInstance->m_fdStdOut, STDOUT_FILENO);
					dup2(s_pInstance->m_fdStdErr, STDERR_FILENO);
					close(s_pInstance->m_fdStdOut);
					close(s_pInstance->m_fdStdErr);
					s_pInstance->m_fdStdOut = 0;
					s_pInstance->m_fdStdErr = 0;
					s_pInstance->m_fdLog = 0;
				}
				break;
		}
	}
	
	int CByVfs::Init(const char* moduleName, const char* pipeName, short port)
	{
		snprintf(CByVfs::CLIHEAD, 30, "\n\033[01;32m%s# \033[01;37m", moduleName);
		CByVfs::CLIHEAD[30] = 0;

		int sock = io_socket_async_create(pipeName);
		if (sock == 0) return -1;
		CByVfs::GetInstance()->m_netloop.AddSvr(sock, CByVfs::onEvent, NULL);
		
		if (port != 0) {
			sock = io_socket_async_create(NULL, port);
			if (sock == 0) return -2;
			CByVfs::GetInstance()->m_netloop.AddSvr(sock, CByVfs::onEvent, NULL);
		}
		return 0;
	}
	
	void CByVfs::Register(const char* name, PrintVfs fun, void* usrdata)
	{
		std::unique_lock<std::mutex> lock(s_lock);
		BYVFS vfs = {fun, usrdata};
		CByVfs::GetInstance()->mFuncMap.insert(std::make_pair(name, vfs));
	}
	
	int CByVfs::onParseMsg(int fd, char* pHead, int len, struct sockaddr* addr)
	{
		int argc = 0;
		char* argv[10] = {NULL};
		SplitToStrs(pHead, len, argc, argv, 10);
		if (argc <= 0) return len;
		
		if (/*(0 == strcmp(argv[0], "help")) || */(0 == strcmp(argv[0], "?"))) {
			SendHelp(fd);
			write(fd, CLIHEAD+1, strlen(CLIHEAD)-1);
		}else if (strcmp(argv[0], "logtome") == 0) {
			if (m_fdStdOut == 0) {
				m_fdStdOut = dup(STDOUT_FILENO);
				m_fdStdErr = dup(STDERR_FILENO);
				
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
			}
			if (m_fdLog > 0) {
				close(m_fdLog);
				
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
			}
			m_fdLog = fd;
			
			int flag = 1;
			setsockopt(STDOUT_FILENO, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
			setsockopt(STDERR_FILENO, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
		}else {
			if (argc && mFuncMap.find(argv[0]) != mFuncMap.end()) {
				BYVFS& vfs = mFuncMap[argv[0]];
				if (vfs.fun){
					(*vfs.fun)(fd, vfs.usrdata, argc, argv);
					write(fd, CLIHEAD, strlen(CLIHEAD));
				}
			}else {
				write(fd, "UnKnow Command\n", strlen("UnKnow Command\n"));
				write(fd, CLIHEAD, strlen(CLIHEAD));
			}
		}
		return len;
	}
	
	char* CByVfs::GetBufInfo(int* unparselen, int* maxlen)
	{
		*unparselen = m_iUnParselen;
		*maxlen = m_iBufferLen;
		return m_pRecvBuffer;
	}
	
	void CByVfs::SetUnParseLen(int unparselen)
	{
		m_pRecvBuffer[unparselen] = 0;
		m_iUnParselen = unparselen;
	}
	
    void CByVfs::SplitToStrs(char* buf, int maxBufLen, int& argc, char** argv, int argNum)
    {
        bool bNewArg = true;
        for (int i = 0; i < maxBufLen && buf[i] != 0 && argc < argNum; i++) {
            if (' ' == buf[i] || '	' == buf[i] || '\n' == buf[i]) {
                bNewArg = true;
                buf[i] = 0;
                continue;
            }

            if (bNewArg) {
                argv[argc] = &buf[i];
                argc++;
                bNewArg = false;
            }
        }
    }

    void CByVfs::SendHelp(int fd)
    {
        #define VFS_HELP_BUFLEN  (1024 * 10)
        char *pcStr = (char *)malloc(VFS_HELP_BUFLEN + 1);
        std::map<std::string, BYVFS>::iterator it;

        unsigned int uiWriteLen = 0, i = 0;
        for (it = mFuncMap.begin(); it != mFuncMap.end(); it++, i++) {
            uiWriteLen += snprintf(pcStr + uiWriteLen, VFS_HELP_BUFLEN - uiWriteLen,"%-15s", it->first.c_str());
            if (i == 8) {
                uiWriteLen += snprintf(pcStr + uiWriteLen, VFS_HELP_BUFLEN - uiWriteLen,"\r\n");
                i = 0;
            }
        }
        uiWriteLen += snprintf(pcStr + uiWriteLen, VFS_HELP_BUFLEN - uiWriteLen,"\r\n");
        write(fd, pcStr, uiWriteLen);
        free(pcStr);
    }
	
	void CByVfs::waitMsg(int ms)
	{
		m_netloop.RunOnce(ms);
	}
	
	extern "C" void InitVfs(const char* moduleName, const char* pipeName, short port)
	{		
		unlink(pipeName); 
		CByVfs::Init(moduleName, pipeName, port);
	}
	
	extern "C" void RegisterVfs(const char* fileName, PrintVfs fun, void* usrdata)
	{
		CByVfs::Register(fileName, fun, usrdata);
	}
	
	extern "C" void WaitVfsMsg(int ms)
	{
		CByVfs::GetInstance()->waitMsg(ms);
	}
	
	extern "C" void RunVfsInNewThread()
	{
		std::thread t([](){
			CByVfs* pVfs = CByVfs::GetInstance();
			while(1) {
				pVfs->waitMsg(1000);
			}
		});
		t.detach();
	}
}
