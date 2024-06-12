#include "platform.h"

#include <mutex>
#include <vector>
#include <algorithm>

#ifdef WIN32
#include <IPTypes.h>
#include <IPHlpApi.h>
//#pragma comment(lib, "ws2_32.lib")
//#pragma comment(lib, "IPHLPAPI.lib")
#include "io_fd_factory.h"

#ifdef __cplusplus
extern "C" {
#endif

class CEpHandle{
public:
	CEpHandle() {}
	~CEpHandle() {}
	
	int add(int fd, struct epoll_event* event) {
		std::unique_lock<std::mutex> lock(m_mutex);
		std::vector<struct epoll_event>::iterator it = std::find(m_vtEvent.begin(), m_vtEvent.end(), fd);
		if (it == m_vtEvent.end()){
			m_vtEvent.push_back(*event);
		}
		return 0;
	}
	
	int del(int fd) {
		std::unique_lock<std::mutex> lock(m_mutex);
		std::vector<struct epoll_event>::iterator it = std::find(m_vtEvent.begin(), m_vtEvent.end(), fd);
		if (it != m_vtEvent.end()){
			m_vtEvent.erase(it);
		}
		return 0;
	}
	
	int wait(struct epoll_event *events, int maxevents, int tm) {
		fd_set fdr;
		FD_ZERO(&fdr);
		
		int maxFd = 0;
		int usrEvSize = 0;
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			usrEvSize = (int)m_vtEvent.size();
			for (int i = 0; i < usrEvSize; i++) {
				FD_SET(m_vtEvent[i].data.fd, &fdr);
				maxFd = (maxFd > m_vtEvent[i].data.fd) ? maxFd : m_vtEvent[i].data.fd;
			}
			if (maxFd == 0) {
				Sleep(10);
				return 0;
			}
		}
		
		int nfds = 0;
		struct timeval timeout = { tm/1000, (tm%1000)*1000};
		int flags = select(maxFd+1, &fdr, NULL, NULL, &timeout);
		for(int i=0; i<usrEvSize && nfds < flags; i++) {
			if (FD_ISSET(m_vtEvent[i].data.fd, &fdr)) {
				memcpy(&events[nfds], &m_vtEvent[i], sizeof(m_vtEvent[i]));
				nfds++;
			}
		}
		return nfds;
	}
	
private:
	std::mutex	m_mutex;
	std::vector<struct epoll_event>	m_vtEvent;
};

int epoll_create(int size)
{
	CEpHandle* pHandle = new CEpHandle();
	return PLATFORM::io_fd_alloc(pHandle);
}

int epoll_ctl(int epfd, int cmd, int fd, struct epoll_event* event)
{
	CEpHandle* pHandle = (CEpHandle*)PLATFORM::io_get_fd(epfd);
	if (!pHandle) return -1;
	
	if (cmd == EPOLL_CTL_ADD) {
		return pHandle->add(fd, event);
	}else if (cmd == EPOLL_CTL_DEL) {
		return pHandle->del(fd);
	}
	return 0;
}

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	CEpHandle* pHandle = (CEpHandle*)PLATFORM::io_get_fd(epfd);
	if (!pHandle) return -1;
	
	return pHandle->wait(events, maxevents, timeout);
}

int epoll_release(int epfd) {
	CEpHandle* pHandle = (CEpHandle*)PLATFORM::io_fd_free(epfd);
	if (!pHandle) return -1;
	delete pHandle;
	return 0;
}

int msgget(key_t __key, int __msgflg)
{
	return 0;
}

int msgsnd (int msqid, const void *msgp, size_t msgsz, int msgflg)
{
	return 0;
}

int msgrcv (int msqid, void *msgp, size_t msgsz, long int msgtyp, int msgflg)
{
	return 0;
}

int getuid(void)
{
	return 0;
}

int getnameinfo (const struct sockaddr *sa, socklen_t addrlen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags)
{
	return 0;
}

void __sk_addrun_init(struct sockaddr_un* addr, const char* pipe)
{
	memset(addr, 0, sizeof(*addr));
	addr->sun_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr("127.0.0.1");
	addr->sin_port = htons(atoi(pipe));
}

int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;

	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;

	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;

	if (tzp) {
		tzp->tz_dsttime = 0;
		tzp->tz_minuteswest = 0;
	}
	return (0);
}

bool __x_isdir(const char* path)
{
	return false;
}

int __x_removedir(const char *dir)
{
	return 0;
}

void __x_copydir(const char * sourcePath, const char * destPath)
{
}

#ifdef __cplusplus
}
#endif

std::vector<struct sockaddr_in> __get_broadcast_addr(unsigned short port) {
	std::vector<struct sockaddr_in> vt;

	PMIB_IPADDRTABLE pIPTable = NULL;
	DWORD dwSize = 0;
	::GetIpAddrTable(pIPTable, &dwSize, TRUE);

	pIPTable = (PMIB_IPADDRTABLE)::GlobalAlloc(GPTR, dwSize);
	::GetIpAddrTable(pIPTable, &dwSize, TRUE);

	char szIPAddr[17], szSubMask[17];
	for (DWORD i = 0; i < pIPTable->dwNumEntries; i++) {
		memset(szIPAddr, 0x0, sizeof(szIPAddr));
		sprintf(szIPAddr, "%d.%d.%d.%d",
			LOWORD(pIPTable->table[i].dwAddr) & 0x00FF,
			LOWORD(pIPTable->table[i].dwAddr) >> 8,
			HIWORD(pIPTable->table[i].dwAddr) & 0x00FF,
			HIWORD(pIPTable->table[i].dwAddr) >> 8);
		if (strcmp("127.0.0.1", szIPAddr) == 0) {
			continue;
		}

		memset(szSubMask, 0x0, sizeof(szSubMask));
		sprintf(szSubMask, "%d.%d.%d.%d",
			LOWORD(pIPTable->table[i].dwMask) & 0x00FF,
			LOWORD(pIPTable->table[i].dwMask) >> 8,
			HIWORD(pIPTable->table[i].dwMask) & 0x00FF,
			HIWORD(pIPTable->table[i].dwMask) >> 8);

		in_addr ipBroadcast;
		ipBroadcast.s_addr = inet_addr(szIPAddr) | (~inet_addr(szSubMask));
		char* sBroadcast = inet_ntoa(ipBroadcast);

		struct sockaddr_in addr;
		memset(&addr, 0x0, sizeof(struct sockaddr_in));

		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = inet_addr(sBroadcast);
		addr.sin_port = htons(port);
		vt.push_back(addr);
	}
	::GlobalFree(pIPTable);
	return vt;
}

#else
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

int closesocket(int sk)
{
	return ::close(sk);
}

void __sk_addrun_init(struct sockaddr_un* addr, const char* pipe)
{
	strcpy(addr->sun_path, pipe);
}

int epoll_release(int epfd) {
	return ::close(epfd);
}

bool __x_isdir(const char* path)
{
	if (strcmp(path, "") == 0) {
		return false;
	}

	struct stat st;
	if (0 != stat(path, &st)) {
		return false;
	}

	if (S_ISDIR(st.st_mode)) {
		return true;
	}

	return false;
}

int __x_removedir(const char *dir)
{
	DIR *dirp;
	struct dirent *dp;
	char dir_name[512];
	struct stat dir_stat;

	if (0 != access(dir, F_OK)) {
		return 0;
	}

	if (0 > stat(dir, &dir_stat)) {
		return -1;
	}

	if (S_ISREG(dir_stat.st_mode)) {
		remove(dir);
	}
	else if (S_ISDIR(dir_stat.st_mode)) {
		dirp = opendir(dir);
		while ((dp = readdir(dirp)) != NULL) {
			if ((0 == strcmp(".", dp->d_name)) || (0 == strcmp("..", dp->d_name))) {
				continue;
			}

			snprintf(dir_name, sizeof(dir_name), "%s/%s", dir, dp->d_name);
			io_file_removedir(dir_name);
		}
		closedir(dirp);
		rmdir(dir);
	}
	return 0;
}

void __x_copydir(const char * sourcePath, const char * destPath)
{
	struct dirent* filename = NULL;
	if (0 == opendir(destPath.c_str())) {
		if (mkdir(destPath.c_str(), 0777)) {
			BYLOG_E("Create Dir Failed...\n");
		}
	}

	std::string path = sourcePath;
	if (sourcePath.back() != '/') {
		sourcePath += "/";
	}

	if (destPath.back() != '/') {
		destPath += "/";
	}

	DIR* dp = opendir(path.c_str());
	while ((filename = readdir(dp))) {
		std::string fileSourceFath = sourcePath;
		std::string fileDestPath = destPath;
		fileSourceFath += filename->d_name;
		fileDestPath += filename->d_name;
		if (io_file_isdir(fileSourceFath.c_str())) {
			if (strncmp(filename->d_name, ".", 1) && strncmp(filename->d_name, "..", 2)) {
				io_file_copydir(fileSourceFath, fileDestPath);
			}
		}
		else {
			io_file_copyfile(fileSourceFath.c_str(), fileDestPath.c_str());
		}
	}
}
#endif