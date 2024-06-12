#ifndef _PLATFORM_BYVFS_H__
#define _PLATFORM_BYVFS_H__

namespace PLATFORM
{
    typedef void (*PrintVfs)(int fd, void* usrdata, int argc, char*argv[]);
	extern "C" void InitVfs(const char* moduleName, const char* pipeName, short port);
	extern "C" void RegisterVfs(const char* fileName, PrintVfs fun, void* usrdata);
	extern "C" void WaitVfsMsg(int ms);
	extern "C" void RunVfsInNewThread();
};
#endif
