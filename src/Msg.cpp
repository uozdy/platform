#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include "Msg.h"
#include "ByLog.h"
#include "platform.h"
namespace PLATFORM
{

int CMsg::get(const char *fName, int flag )
{
    return msgget( 0x2, flag );
}

int CMsg::send(int msgid, msgbuf_t *pMsg)
{
    if ( !pMsg )
    {
        return -1;
    }
    return msgsnd( msgid, pMsg, sizeof(msgbuf_t) - sizeof(long), IPC_NOWAIT );
}

int CMsg::rcev(int msgid, msgbuf_t *pMsg, long type)
{
    if ( !pMsg )
    {
        return -1;
    }
    return msgrcv( msgid, pMsg, sizeof(msgbuf_t), type, IPC_NOWAIT );
}

GtMsg CMsg::getGtMsg(char* key, long type, int id, char* name)
{
	GtMsg gt;
	MsgBody_t *pBody = (MsgBody_t *)gt.msg.data;
	gt.msg.mtype = type;
	pBody->msgid = id;
	pBody->proc.pid = GetCurrentProcessId();
	strcpy( pBody->proc.name, name );
	gt.msgid = CMsg::get( key, IPC_CREAT | 0666 );
	return gt;
}
}
