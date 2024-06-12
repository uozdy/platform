#ifndef __SRC_ADV_AES_H__
#define __SRC_ADV_AES_H__

namespace PLATFORM {

    int ALG_AES_EnCrypt(const char* input, int slen, const char* key, int keysize, char* output);
    int ALG_AES_DeCrypt(const char* input, int slen, const char* key, int keysize, char* output); 

}
#endif
