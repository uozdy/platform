#ifndef __IO_FD_FACTORY__
#define __IO_FD_FACTORY__

namespace PLATFORM {
	int io_fd_alloc(void* pUsr);
	void* io_fd_free(int fd);
	void* io_get_fd(int fd);
}

#endif
