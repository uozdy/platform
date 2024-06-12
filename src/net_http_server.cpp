#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <map>
#include <mutex>
#include "net_http_server.h"
#include "std_string_ext.h"
#include "io_socket_ext.h"
#include "ByTime.h"
#include "platform.h"

namespace PLATFORM
{
    class CHttpServer;
    enum {
        PARSEST_REQUEST,
        PARSEST_REQUEST_END,
        PARSEST_HEAD,
        PARSEST_CONTENT,
        PARSEST_CONTENT_BOUNDARY,
        PARSEST_RESPONSE,
        PARSEST_END,
    };

    enum {
        PARSEBOUNDARY_CHECK,
        PARSEBOUNDARY_HEAD,
        PARSEBOUNDARY_CONTENT,
        PARSEBOUNDARY_CONTENT_FILE,
        PARSEBOUNDARY_END,
    };
    
    typedef void (*CheckBoundaryCall)(HttpParse* self, char* pBuffer, int len);
    #define CheckFuncCall() [](HttpParse* self, char* pBuffer, int len)

    class HttpParse: public IParse {
    public:
        HttpParse(int fd, CHttpServer* pOwner, CNetTcpLoop* pAtachLooper);
        virtual ~HttpParse();

        void toResponse(REQUEST* request, RESPONSE* response);
    protected:
        virtual int onParseMsg(int fd, char* pBuffer, int len, struct sockaddr* addr);
        virtual char* GetBufInfo(int* unparselen, int* maxlen);
        virtual void SetUnParseLen(int unparselen);
        virtual bool IsQuit();
        virtual bool WriteData(int fd);
        void ComitResponse(std::string contentType, bool bW);

    private:
        void ParseHttpMethod(char* pBuffer);
        void ParseHttpHead(char* pBuffer, char* pLineEnd);
        int CheckBoundary(CheckBoundaryCall func, char* pBuffer, int len, char* pLineEnd);
        int ParseBoundaryHead(char* pBuffer, char* pLineEnd);
        int ParseContentFile(char* pBuffer, int len);
    private:
        short           m_iParseSt;
        int             m_iFd;

		char*			m_pRecvBuffer;
		int				m_iUnParselen;
		int				m_iBufferLen;
        
        CHttpServer*    m_pOwner;
		CNetTcpLoop*    m_pAtachLooper;
        void*           m_pEventPtr;
        REQUEST         m_stRequest;
        RESPONSE        m_stResponse;

        bool            m_bComitted;
        short           m_iParseBoundary;
        std::string     m_strBoundary;
        FILE*           m_hCurrentFile;
        int             m_iBoundaryLen;

        int             m_iResponseOffset;
        bool            m_bQuit;
		friend class    CHttpServer;
    };

    HttpParse::HttpParse(int fd, CHttpServer* pOwner, CNetTcpLoop* pAtachLooper)
        : m_iParseSt(PARSEST_REQUEST)
        , m_iFd(fd)
        , m_iUnParselen(0)
        , m_pOwner(pOwner)
        , m_pAtachLooper(pAtachLooper)
        , m_hCurrentFile(NULL)
        , m_iResponseOffset(0)
        , m_bQuit(false)
    {
        m_iBufferLen = 2*1024;
        m_pRecvBuffer = (char*)malloc(m_iBufferLen);
        m_stResponse.code = 0;
		m_stResponse.connection = CONNECTION_CLOSE;
		m_stResponse.pContentData = NULL;
		m_stResponse.pContentLen = 0;
		m_stResponse.pCacheSize = 0;
		m_stResponse.contentType.clear();
		m_stResponse.header.clear();
        m_stResponse.submit = [this](bool bw){
            ComitResponse(this->m_stResponse.contentType, bw);
        };
    }
    
    HttpParse::~HttpParse()
    {
        free(m_pRecvBuffer);
        m_pRecvBuffer = NULL;

        if (m_stResponse.pContentData) {
            free(m_stResponse.pContentData);
        }
        m_stResponse.pContentData = NULL;
        m_stResponse.pCacheSize = 0;
    }

    bool HttpParse::IsQuit() 
    {
        return m_bQuit;
    }
    
    int HttpParse::ParseBoundaryHead(char* pBuffer, char* pLineEnd)
    {
        pLineEnd[0] = 0;
        if(pLineEnd == pBuffer || (pLineEnd == &pBuffer[1] && pBuffer[0] == '\r')) {
            if (m_hCurrentFile) m_iParseBoundary = PARSEBOUNDARY_CONTENT_FILE;
            else m_iParseBoundary = PARSEBOUNDARY_CONTENT;
        }else {
            char key[32] = {0};
            char value[128] = {0};
            if ( 2 == sscanf (pBuffer, "%31[^:^ ]%*[: ]%127s", key, value) ) {
                if (strcasecmp(key, "Content-Disposition") == 0 ) {
                    char name[32] = {0};
                    char file[128] = {0};
                    std::string str = pBuffer;
                    std::vector<std::string> vt = std_string_splitString(str, ";");
                    for (unsigned int i = 1; i < vt.size(); i++) {
                        if (1 == sscanf (vt[i].c_str(), " name=\"%31[^\"]", name)) {
                            if (!m_stRequest.content.empty()) 
                                m_stRequest.content += "&";
                            m_stRequest.content += name;
                            m_stRequest.content += "=";
                        }else if(1 == sscanf (vt[i].c_str(), " filename=\"%127[^\"]", file)) {
                            std::string f;
                            std_string_format(f, "%llu_%s", CTime::GetUTCTime_Ms(), file);
                            m_stRequest.content += f;
                            assert(m_hCurrentFile == NULL);

                            std::string fPath = m_pOwner->m_strUploadDir.c_str();
                            fPath += "/";
                            fPath += f;
                            m_hCurrentFile = fopen(fPath.c_str(), "wb");
                        } 
                    }                             
                }else if (strcasecmp(key, "content-type") == 0) {
                                
                }           
            }
        }
        return pLineEnd + 1 - pBuffer;
    }

    void HttpParse::ParseHttpMethod(char* pBuffer)
    {
        char method[10] = {0};
        char url[128] = {0};
        char version[10] = {0};
        if ( 3 == sscanf(pBuffer, "%10s %128s %20s", method, url, version) ) {
            if (strcasecmp(method, "get") == 0) m_stRequest.method = HTTPMETHOD_GET;
            else if (strcasecmp(method, "post") == 0) m_stRequest.method = HTTPMETHOD_POST;

            m_stRequest.url = url;
            m_stRequest.param.clear();
            m_stRequest.content.clear();
            m_stRequest.connection = 0;
            m_stRequest.contentlen = 0;
            m_iParseSt = PARSEST_HEAD;
            m_strBoundary.clear();
            m_iBoundaryLen = 0;
            m_stResponse.contentType.clear();
		    m_stResponse.header.clear();

            std::vector<std::string> vt = std_string_splitString(m_stRequest.url, "?");
            if (vt.size() == 2) {
                m_stRequest.url = vt[0];
                m_stRequest.param = vt[1];
            }
        }
    }

    void HttpParse::ParseHttpHead(char* pBuffer, char* pLineEnd)
    {
        char key[32] = {0};
        char value[1024] = {0};
        if ( 2 == sscanf (pBuffer, "%31[^:^ ]%*[: ]%1023[^\n^\r]", key, value) ) {
            if (strcasecmp(key, "connection") == 0) {
                if (strcasecmp(value, "close") == 0) m_stRequest.connection = CONNECTION_CLOSE;
                else if (strcasecmp(value, "keep-alive") == 0) m_stRequest.connection = CONNECTION_KEEPALIVE;
            } else if (strcasecmp(key, "content-length") == 0) {
                m_stRequest.contentlen = atoi(value);
            } else if (strcasecmp(key, "content-type") == 0) {
                std::string contentType = value;
                if (std_string_indexOf(contentType, "multipart/form-data") == 0) {
                    char boundary[32] = {0};
                    char info[128] = {0};
                    if ( 2 == sscanf (pBuffer, "%*[^;]; %31[^=]=%127s", boundary, info) ) {
                        m_strBoundary = "--";
                        m_strBoundary += info;
                        m_iParseBoundary = PARSEBOUNDARY_CHECK;
                    }
                }
            } else if (strcasecmp(key, "cookie") == 0) {
                m_stRequest.cookie = value;
            }
        }else if(pLineEnd == pBuffer || (pLineEnd == &pBuffer[1] && pBuffer[0] == '\r')){
            if (m_stRequest.method == HTTPMETHOD_GET) m_iParseSt = PARSEST_REQUEST_END;
            else if (m_stRequest.method == HTTPMETHOD_POST) {
                if (m_strBoundary.length() == 0) m_iParseSt = PARSEST_CONTENT;
                else m_iParseSt = PARSEST_CONTENT_BOUNDARY;
            }
        }
    }

    int HttpParse::ParseContentFile(char* pBuffer, int len)
    {
        char* pLineEnd = std_strchr(pBuffer, '\n', len);
        if ( pLineEnd ) {
            return CheckBoundary(CheckFuncCall() {
                if (self->m_hCurrentFile){
                    fwrite(pBuffer, len, 1, self->m_hCurrentFile);
                    self->m_iBoundaryLen += len;
                }
            }, pBuffer, len, pLineEnd);
        }else {
            if (m_hCurrentFile) {
                fwrite(pBuffer, len, 1, m_hCurrentFile);
                m_iBoundaryLen += len;
            }else {
                assert(0);
            }
        }
        return len;
    }

    int HttpParse::CheckBoundary(CheckBoundaryCall func, char* pBuffer, int len, char* pLineEnd)
    {
        if (std_string_fixComp(m_strBoundary, pBuffer, m_strBoundary.length())) {
            m_iParseBoundary = PARSEBOUNDARY_HEAD;
            if (m_hCurrentFile) {
                fclose(m_hCurrentFile);
                m_hCurrentFile = NULL;
            }

            if ( (pLineEnd[-1] == '\r' && pLineEnd[-2] == '-' && pLineEnd[-3] == '-') 
                    || (pLineEnd[-1] == '-' && pLineEnd[-2] == '-') ) {
                m_iParseBoundary = PARSEBOUNDARY_END;
                m_iParseSt = PARSEST_REQUEST_END;
            }
            return pLineEnd + 1 - pBuffer;
        }else if (len > 0) {
            int dlen = len - (pLineEnd+1-pBuffer);
            if (dlen >= (int)m_strBoundary.length()) {
                int parselen = pLineEnd + 1 - pBuffer;
                if (std_string_fixComp(m_strBoundary, pLineEnd+1, m_strBoundary.length())) {
                    parselen --;
                    if (pLineEnd[-1] == '\r') parselen--;
                }

                func(this, pBuffer, parselen);
                return pLineEnd + 1 - pBuffer;
            }
        }else {
            func(this, pBuffer, len);
        }
        return 0;
    }

    int HttpParse::onParseMsg(int fd, char* pBuffer, int len, struct sockaddr* addr)
    {
        if (PARSEST_CONTENT_BOUNDARY == m_iParseSt) {
            int iParseLen = 0;
            if (PARSEBOUNDARY_CHECK == m_iParseBoundary) {
                char* pLineEnd = strchr(pBuffer, '\n');
                if ( pLineEnd ) {
                    iParseLen = CheckBoundary(CheckFuncCall() {
                        assert(0);
                    }, pBuffer, 0, pLineEnd);
                }
            }else if (PARSEBOUNDARY_HEAD == m_iParseBoundary) {
                char* pLineEnd = strchr(pBuffer, '\n');
                if ( pLineEnd ) {
                    iParseLen = ParseBoundaryHead(pBuffer, pLineEnd);
                }
            }else if (PARSEBOUNDARY_CONTENT == m_iParseBoundary) {
                char* pLineEnd = strchr(pBuffer, '\n');
                if ( pLineEnd ) {
                    iParseLen = CheckBoundary(CheckFuncCall() {
                        char c = pBuffer[len];
                        pBuffer[len] = 0;
                        self->m_stRequest.content += pBuffer;
                        self->m_iBoundaryLen += strlen(pBuffer);
                        pBuffer[len] = c;
                    }, pBuffer, len, pLineEnd);
                }
            }else if (PARSEBOUNDARY_CONTENT_FILE == m_iParseBoundary) {
                iParseLen = ParseContentFile(pBuffer, len);
            }

            if (m_iParseSt == PARSEST_REQUEST_END) {
                m_iParseSt = PARSEST_END;
                toResponse(&m_stRequest, &m_stResponse);
            }
            return iParseLen;
        }else if (PARSEST_CONTENT == m_iParseSt) {
            m_stRequest.content += pBuffer;
            if ((int)m_stRequest.content.length() >= m_stRequest.contentlen) {
                m_iParseSt = PARSEST_END;
                toResponse(&m_stRequest, &m_stResponse);
            }
            return len;
        }

        char* pLineEnd = strchr(pBuffer, '\n');
        if ( pLineEnd ) {
            pLineEnd[0] = 0;

            if (PARSEST_REQUEST == m_iParseSt) {
                ParseHttpMethod(pBuffer);
            }else if (PARSEST_HEAD == m_iParseSt){
                ParseHttpHead(pBuffer, pLineEnd);
            }

            if (m_iParseSt == PARSEST_REQUEST_END) {
                m_iParseSt = PARSEST_END;
                toResponse(&m_stRequest, &m_stResponse);
            }
            return pLineEnd + 1 - pBuffer;
        }

        return 0;
    }

    void HttpParse::toResponse(REQUEST* request, RESPONSE* response)
    {
        request->url = std_string_urlDecode(request->url);

        m_bComitted = false;
        bool isStaticRes = m_pOwner->onFliter(request, response);
        if (m_bComitted) {
            return;
        }

        std::string contentType = "text/html";

        if (request->url == "/") request->url = "/index.html";
        if (std_string_endWith(request->url, ".html")) contentType = "text/html";
		else if (std_string_endWith(request->url, ".css")) contentType = "text/css";
		else if (std_string_endWith(request->url, ".js")) contentType = "text/javascript";
		else if (std_string_endWith(request->url, ".png")) contentType = "image/png";
		else if (std_string_endWith(request->url, ".jpg")) contentType = "image/jpeg";
		else if (std_string_endWith(request->url, ".wav")) contentType = "audio/x-wav";
		else if (std_string_endWith(request->url, ".json")) contentType = "application/json";
		else if (std_string_endWith(request->url, ".ico")) contentType = "image/x-icon";
		else if (std_string_endWith(request->url, ".woff")) contentType = "font/woff";
		else isStaticRes = false;

        response->code = 404;
        response->connection = m_stRequest.connection;
        response->pContentLen = 0;
        if (isStaticRes) {
            int len = 0;
            std::string filename = m_pOwner->m_strDocRoot + request->url;
            FILE* fp = fopen(filename.c_str(), "rb");
            if (fp) {
                fseek(fp, 0x00, SEEK_END);
                len = ftell(fp);
                fseek(fp, 0x00, SEEK_SET);
                response->code = 200;
            }
            
            RESPONSECACHE(response, len);
            if (fp) {
                response->pContentLen = len;
                fread(response->pContentData+MAXRESPONSEHEADLEN, len, 1, fp);
                fclose(fp);
            }

            ComitResponse(contentType, true);
        }else{
            m_pOwner->onRequest(request, response);
        }
    }

    void HttpParse::ComitResponse(std::string contentType, bool bW)
    {
        m_bComitted = true;
        std::string res = "HTTP/1.1 ";
        res += std_string_intToString(m_stResponse.code);
        res += " OK\r\n";
        //res += "host:127.0.0.1\r\n";
        res += "content-length:";
        res += std_string_intToString(m_stResponse.pContentLen);
        res += "\r\n";
        res += m_stResponse.header;
        res += "connection:";
        res += (m_stResponse.connection==CONNECTION_KEEPALIVE)?"keep-alive":"close";
        res += "\r\n";
        res += "content-type:";
        res += contentType;
        res += "\r\n\r\n";

        m_iResponseOffset = MAXRESPONSEHEADLEN-res.length();
        m_stResponse.pContentLen += MAXRESPONSEHEADLEN;
        strncpy(m_stResponse.pContentData+m_iResponseOffset, res.c_str(), res.length());
        
        if (!bW) m_pAtachLooper->ChgClt(m_iFd, m_pEventPtr, true);
        else WriteData(m_iFd);
    }

    bool HttpParse::WriteData(int fd)
    {
        while(m_iResponseOffset < m_stResponse.pContentLen) {
            int ret = send(m_iFd, m_stResponse.pContentData+m_iResponseOffset, m_stResponse.pContentLen-m_iResponseOffset, 0);
            if (ret > 0) {
                m_iResponseOffset += ret;
                if (m_iResponseOffset == m_stResponse.pContentLen) break;
            }else {
                break;
            }
        }
        
        bool bWrite = m_stResponse.pContentLen > m_iResponseOffset;
        m_pAtachLooper->ChgClt(m_iFd, m_pEventPtr, bWrite);
        if (!bWrite) {
            m_iParseSt = PARSEST_REQUEST;
            if (m_stResponse.connection == CONNECTION_CLOSE) {
                m_bQuit = true;
                return false;
            }
        }

        return true;
    }

    char* HttpParse::GetBufInfo(int* unparselen, int* maxlen)
    {
        *unparselen = m_iUnParselen;
		*maxlen = m_iBufferLen;
		return m_pRecvBuffer;
    }
    
    void HttpParse::SetUnParseLen(int unparselen)
    {
        m_pRecvBuffer[unparselen] = 0;
		m_iUnParselen = unparselen;
    }
	
    CHttpServer::CHttpServer(int port, std::string documentRoot, std::string uploadDir)
        : m_strDocRoot(documentRoot)
        , m_strUploadDir(uploadDir)
    {
        m_iSvrSk = io_socket_async_create(NULL, port);
        assert(m_iSvrSk);

        m_netloop.AddSvr(m_iSvrSk, [](int fd, int msg, void* wParam, void* lParam) {
            CHttpServer* pServer = (CHttpServer*)lParam;
            int index = fd % TASKCOUNT;
			CNetTcpLoop* pNetLoop = &pServer->m_clientloop[index];
            HttpParse* pParse = new HttpParse(fd, pServer, pNetLoop);
            pParse->m_pEventPtr = pNetLoop->AddClt(fd, [](int fd, int msg, void* wParam, void* lParam) {
                if (wParam) delete (HttpParse*)wParam;
				closesocket(fd);
            }, [](int socket, char* pRecvBuf, int iMaxSize, struct sockaddr* addr)->int {
                return recv(socket, pRecvBuf, iMaxSize, 0);
            }, pParse, pServer); 
        }, this);

        m_bClientRun = true;
        for (int i = 0; i < TASKCOUNT; i++) {
            m_pThread[i] = m_clientloop[i].RunInNewThread(100, &m_bClientRun);
        }
    }

	CHttpServer::~CHttpServer()
    {
        m_bClientRun = false;
        for (int i = 0; i < TASKCOUNT; i++) {
            m_pThread[i]->ExitThread();
        }
        closesocket(m_iSvrSk);
    }

    std::map<std::string, std::string> CHttpServer::ParseParam(std::string param, const char* split)
    {
        std::map<std::string, std::string> map;
        std::vector<std::string> vt = std_string_splitString(param, split);
        for (unsigned int i = 0; i < vt.size(); i++)
        {
            char key[64]={0}, value[512]={0};
            if (sscanf(vt[i].c_str(), " %63[^=]= %511[^\n^\r]", key, value) == 2) {
                map.insert(std::make_pair(key, value));
            }
        }
        return map;
    }

    void CHttpServer::Redirect(RESPONSE* response, std::string url)
    {
        response->connection = CONNECTION_KEEPALIVE;
        response->code = 302;
        response->pContentLen = 0;
        //response->url = url;
		response->header += ("Location:" + url + "\r\n");
        RESPONSECACHE(response, 0);
        response->submit(true);
    }

    void CHttpServer::waitMsg(int ms)
    {
        m_netloop.RunOnce(ms);
    }
}
