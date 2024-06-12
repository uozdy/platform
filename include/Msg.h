#ifndef COMMONLIB_CMSG_H_
#define COMMONLIB_CMSG_H_
#include <fcntl.h>

namespace PLATFORM
{

typedef struct _process_
{
    int   pid;
    char  name[64];
} process_t;

typedef struct _MsgBody_
{
    int msgid;
    union
    {
        process_t proc;
    };
} MsgBody_t;

typedef struct _msgbuf_
{
    long mtype;
    char data[256];
} msgbuf_t;

typedef struct{
	int 		msgid;
	msgbuf_t 	msg;
}GtMsg;

class CMsg
{
public:
	static int get(const char *fName, int flag );
	static int send(int msgid, msgbuf_t *pMsg);
	static int rcev(int msgid, msgbuf_t *pMsg, long type);
	
	static GtMsg getGtMsg(char* key, long type, int id, char* name);
};

}
#endif /* COMMONLIB_CMSG_H_ */
