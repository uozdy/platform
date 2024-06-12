#include <stdio.h>
#include <string.h>

#include "platform.h"

#include "ByLog.h"
#include "io_file_ext.h"
extern int mkdir(const char*, int);

namespace PLATFORM {
	
int io_file_removedir(const char *dir)
{
	return __x_removedir(dir);
}


int io_file_copyfile(const char* src, const char* des)
{
	int nLen = 0;
	char szBuf[1024] = {0};

	FILE* pSrc = fopen(src, "r");
	if (!pSrc) {
		BYLOG_E("Can't open('r') file(%s)\n", src);
		return -1;
	}

	FILE* pDes = fopen(des, "wb");
	if (!pDes) {
		BYLOG_E("Can't open('wb') file(%s)\n", des);
		fclose(pSrc);
		return -1;
	}
		
	while((nLen = fread(szBuf, 1, sizeof szBuf, pSrc)) > 0) {
		fwrite(szBuf, 1, nLen, pDes);
	}

	fclose(pSrc);
	fclose(pDes);
	return 0;
}

bool io_file_isdir(const char* path)
{
	return __x_isdir(path);
}

void io_file_copydir(std::string sourcePath, std::string destPath)
{
	__x_copydir(sourcePath.c_str(), destPath.c_str());
}

}
