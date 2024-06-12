
#ifndef __NOTIFICATIONCENTER_H__
#define __NOTIFICATIONCENTER_H__

#include <map>
#include <vector>
#include <functional>
#include "MsgThreadPool.hpp"

namespace PLATFORM {

typedef struct{
    int 		NotifyID;
    std::string data;
}NotifyMSG;
typedef std::vector<NotifyMSG> VNotifyMSG;
typedef std::function<void(long long usrID, NotifyMSG* msg)> NotifyFn;

typedef struct{
    long long   usrID;
    NotifyFn    fn;
}ObserverInfo;
typedef std::vector<ObserverInfo> VObserverInfo;

class CNotificationCenter
{
public:
    static CNotificationCenter* defaultCenter(int index=0);
    void addObserver(long long usrID, int NotifyID, NotifyFn fn);
    void notify(NotifyMSG* msg);

private:
    CNotificationCenter(){};
    ~CNotificationCenter(){};
    static void* NotifyThread(void* param);
    
private:
    CMsgThreadPool<NotifyMSG, VNotifyMSG> m_thread;
    std::map<int, VObserverInfo>  m_observers;
};

}

#endif