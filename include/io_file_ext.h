#ifndef __IO_FILE_EXT_H__
#define __IO_FILE_EXT_H__
#include <string>

namespace PLATFORM {
	bool io_file_isdir(const char* path);
	int io_file_removedir(const char *dir);
	void io_file_copydir(std::string sourcePath, std::string destPath);
}
#endif