#ifndef __PLATFORM_SRC_NET_JWT__
#define __PLATFORM_SRC_NET_JWT__
#include <string>
#include <functional>
namespace PLATFORM {
    typedef std::function<void(bool ret, std::string payload)> VERIFYCALLFN;

    enum {
        JWTENC_NONE,
        JWTENC_AES,
    };

    std::string net_jwt_signature(int encrypt, std::string payload, std::string secret);
    bool net_jwt_verify(std::string token, std::string secret);
    void net_jwt_verify(std::string token, std::string secret, VERIFYCALLFN fn);
}

#endif