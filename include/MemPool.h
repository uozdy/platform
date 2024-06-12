#ifndef __PLATFORM_SRC_MEMPOOL__
#define __PLATFORM_SRC_MEMPOOL__
#include <stdio.h>
#include <stdlib.h>

namespace PLATFORM {

bool MP_Init(int idx, int cursize, int maxsize);
bool MP_DeInit(int idx);
void* MP_malloc(int idx, int len);
bool MP_free(int idx, void* data);
void PrintNode(int idx);

}

#endif