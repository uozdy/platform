#include <string.h>
#include <assert.h>
#include "net_jwt.h"
#include "Base64.h"
#include "cJSON.h"
#include "std_string_ext.h"
#include "algorithms/AES.h"
#include "platform.h"

namespace PLATFORM
{
    typedef int (*_jwtencryptfunc)(const char* input, int slen, const char* key, int keysize, char* output);

    static const char* _jwtEncToString(int encrypt)
    {
        switch (encrypt) {
        case JWTENC_AES:
            return "{\"alg\":\"aes\"}";
        }
        return "{\"alg\":\"none\"}";
    }

    static _jwtencryptfunc _jwtEncFunc(int encrypt) 
    {
        switch (encrypt) {
        case JWTENC_AES:
            return ALG_AES_EnCrypt;
        }
        return NULL;
    }

    static _jwtencryptfunc _jwtDecFunc(char* encrypt) {
        if (strcasecmp(encrypt, "aes") == 0) {
            return ALG_AES_DeCrypt;
        }
        return NULL;
    }

    std::string net_jwt_signature(int encrypt, std::string payload, std::string secret)
    {
        _jwtencryptfunc func = _jwtEncFunc(encrypt);
        assert(func);

        const char* encryptStr = _jwtEncToString(encrypt);
        int len1 = strlen(encryptStr);
        int len2 = cJSON_Minify((char*)payload.c_str());

        std::string headBase64, payloadBase64;
        Base64_Encode2(encryptStr, len1, headBase64);
        Base64_Encode2(payload.c_str(), len2, payloadBase64);

        std::string out = headBase64 + "." + payloadBase64;

        std::string algout;
        algout.resize(out.length() + (16 - (out.length() & 15)));
        
        int len = func(out.c_str(), out.length(), secret.c_str(), secret.length(), (char*)algout.c_str());

        std::string sign;
        Base64_Encode2(algout.c_str(), len, sign);

        return out + "." + sign;
    }

    static bool _jwt_verify(std::string token, std::string secret, std::string* pPayload)
    {
        std::vector<std::string> vt = std_string_splitString(token, ".");
        if (vt.size() != 3) {
            return false;
        }

        std::string header;
        Base64_Decode2(vt[0].c_str(), vt[0].length(), header);

        cJSON* json = cJSON_Parse(header.c_str());
        if (!json) {
			return false;
		}

        cJSON* algmethod = cJSON_GetObjectItem(json, "alg");
        if (algmethod->type != cJSON_String || !algmethod->valuestring) {
            return false;
        }

        _jwtencryptfunc func = _jwtDecFunc(algmethod->valuestring);
        if (!func) {
            return false;
        }

        std::string algout;
        std::string& sign = vt[2];
        Base64_Decode2(sign.c_str(), sign.length(), algout);

        std::string out;
        out.resize(algout.length());
        int len = func(algout.c_str(), algout.length(), secret.c_str(), secret.length(), (char*)out.c_str());
        if (len > 0 && std_string_fixComp(token, out.c_str(), len)) {
            if (pPayload) *pPayload = vt[1];
            return true;
        }
        return false;
    }
    
    bool net_jwt_verify(std::string token, std::string secret)
    {
        return _jwt_verify(token, secret, NULL);
    }

    void net_jwt_verify(std::string token, std::string secret, VERIFYCALLFN fn)
    {
        std::string out;
        if (_jwt_verify(token, secret, &out)) {
            std::string payload;
            Base64_Decode2(out.c_str(), out.length(), payload);
            fn(true, payload);
        }else{
            fn(false, "");
        }
    }
}