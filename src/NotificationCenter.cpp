#include "NotificationCenter.h"
#include <assert.h>

namespace PLATFORM {

CNotificationCenter* CNotificationCenter::defaultCenter(int index)
{
	assert(index >=0 && index < 8);
    static CNotificationCenter* sNotifyCenter[8] =  {NULL};
    if (!sNotifyCenter[index]) {
		char buf[32];
		sprintf(buf, "NotificationCenter-%d", index);
        sNotifyCenter[index] = new CNotificationCenter();
        sNotifyCenter[index]->m_thread.StartSv(1, 1, 1, 512*1024, NotifyThread, sNotifyCenter[index], buf);
    }
    return sNotifyCenter[index];
}

void CNotificationCenter::addObserver(long long usrID, int NotifyID, NotifyFn fn)
{
    ObserverInfo osi;
    osi.usrID = usrID;
    osi.fn = fn;
    m_observers[NotifyID].push_back(osi);
}

void CNotificationCenter::notify(NotifyMSG* msg)
{
    m_thread.NotifyMsg(*msg);
}

void* CNotificationCenter::NotifyThread(void* param)
{
	CNotificationCenter* pNotifyCenter = (CNotificationCenter*)param;
	
	NotifyMSG msg;
	while(true) {
		pNotifyCenter->m_thread.Wait();
		if (pNotifyCenter->m_thread.PeekMsg(msg)) {
            std::map<int, VObserverInfo>::iterator it = pNotifyCenter->m_observers.find(msg.NotifyID);
            if (it != pNotifyCenter->m_observers.end()) {
                VObserverInfo& vMsg = it->second;
                VObserverInfo::iterator pos = vMsg.begin();
                for ( ; pos != vMsg.end(); ++pos) {
                    ObserverInfo& osi = *pos;
                    osi.fn(osi.usrID, &msg);
                } 
            }
		}
	}
	return NULL;
}

}