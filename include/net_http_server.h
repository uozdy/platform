
#ifndef __NETHTTPSERVER_H__
#define __NETHTTPSERVER_H__
#include <string>
#include <vector>
#include <map>
#include <functional>

#include "net_tcp_loop.h"

namespace PLATFORM {
	enum {
        HTTPMETHOD_GET,
        HTTPMETHOD_POST,
        HTTPMETHOD_PUT,
        HTTPMETHOD_SET,
        HTTPMETHOD_DELETE,
    };

	enum {
        CONNECTION_CLOSE,
        CONNECTION_KEEPALIVE,
    };

	typedef struct{
		short	method;
		short	connection;
		int		contentlen;
		std::string url;
		std::string param;
		std::string content;
		std::string cookie;
	}REQUEST;

	typedef struct{
		short	code;
		short	connection;
		char*	pContentData;
		int		pContentLen;
		int		pCacheSize;
		std::string contentType;
		std::string header;
		std::function<void(bool)> submit;
	}RESPONSE;

    #define MAXRESPONSEHEADLEN     512
	#define RESPONSECACHE(response, len)	\
		{\
			int tLen = (len + 1023) / 1024 * 1024 + MAXRESPONSEHEADLEN;\
			if (response->pCacheSize < tLen) {\
                response->pCacheSize = tLen;\
                if (response->pContentData) free(response->pContentData);\
                response->pContentData = (char*)malloc(response->pCacheSize);\
            }\
		}

	typedef void (*OnRequest)(REQUEST* request, RESPONSE* response);
	typedef bool (*OnFliter)(REQUEST* request, RESPONSE* response);

	#define  TASKCOUNT		3

	class HttpParse;
    class CHttpServer {
	public:
		CHttpServer(int port, std::string documentRoot, std::string uploadDir);
		~CHttpServer();
		void waitMsg(int ms);

	public:
		OnRequest	onRequest;
		OnFliter	onFliter;
		static std::map<std::string, std::string> ParseParam(std::string param, const char* split);
		static void Redirect(RESPONSE* response, std::string url);

	private:
		int				m_iSvrSk;
		CNetTcpLoop		m_netloop;
		bool			m_bClientRun;
		CNetTcpLoop		m_clientloop[TASKCOUNT];
		CThread*		m_pThread[TASKCOUNT];
		std::string		m_strDocRoot;
		std::string		m_strUploadDir;
		friend class HttpParse;
	};
}
#endif