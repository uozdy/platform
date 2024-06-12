#include "io_fd_factory.h"
#include <mutex>
#include <queue>
#include <map>

namespace PLATFORM {
	
static int _sAcq = 0;
static std::mutex _sMtx_;
static std::queue<int> _sFree_;
static std::map<int, void*> _sMap_;

int io_fd_alloc(void* pUsr) {
	std::unique_lock<std::mutex> lock(_sMtx_);
	if (_sFree_.empty()) {
		++_sAcq;
		_sMap_.insert(std::make_pair(_sAcq, pUsr));
		return _sAcq;
	}
	int fd = _sFree_.front();
	_sFree_.pop();
	_sMap_.insert(std::make_pair(fd, pUsr));
	return fd;
}

void* io_fd_free(int fd) {
	std::unique_lock<std::mutex> lock(_sMtx_);
	_sFree_.push(fd);
	auto it = _sMap_.find(fd);
	if (it != _sMap_.end()) {
		return it->second;
	}
	return NULL;
}

void* io_get_fd(int fd)
{
	std::unique_lock<std::mutex> lock(_sMtx_);
	auto it = _sMap_.find(fd);
	if (it != _sMap_.end()) {
		return it->second;
	}
	return NULL;
}

}
