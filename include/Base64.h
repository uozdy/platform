#ifndef __ENCRYPT_BASE64_H__
#define __ENCRYPT_BASE64_H__

#include <string>

namespace PLATFORM {

int Base64_Encode(const char *indata, int inlen, char *outdata, int *outlen);
int Base64_Decode(const char *indata, int inlen, char *outdata, int *outlen);


int Base64_Encode2(const char *indata, int inlen, std::string& out);
int Base64_Decode2(const char *indata, int inlen, std::string& out);

}

#endif
