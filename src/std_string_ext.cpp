#include "std_string_ext.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace PLATFORM
{
	
static int _vscprintf(const char * format, va_list pargs)
{ 
	int retval; 
	va_list argcopy; 
	va_copy(argcopy, pargs); 
	retval = vsnprintf(NULL, 0, format, argcopy); 
	va_end(argcopy); 
	return retval; 
}
   
std::string& std_string_format(std::string & _str, const char * _Format, ...)
{
	va_list marker;
	va_start(marker, _Format);
 
	int num_of_chars = _vscprintf(_Format, marker);
    _str.resize(num_of_chars);
 
	vsprintf((char *) _str.c_str(), _Format, marker);
 
	va_end(marker);
	return _str;
}

std::string std_string_intToString(int i)
{
    char buf[20];
    sprintf(buf, "%d", i);
    return buf;
}

std::string std_string_format(const char * _Format, ...)
{
	std::string _str;
	va_list marker;
	va_start(marker, _Format);

	int num_of_chars = _vscprintf(_Format, marker);
	_str.resize(num_of_chars);

	vsprintf((char *)_str.c_str(), _Format, marker);

	va_end(marker);
	return _str;
}

std::vector<std::string> std_string_splitString(std::string& str, const char* split)
{
    std::vector<std::string> vt;
    const char* start = str.c_str();
    const char* pstr = start;
    while(pstr) {
        const char* p = strstr(pstr, split);
        if (!p) {
            if (strcmp(pstr, "") != 0)
                vt.push_back(pstr);
            break;
        }

        vt.push_back(str.substr(pstr-start, p - pstr));
        pstr = p + strlen(split);
    }
    return vt;
}

std::vector<std::string> std_string_splitsString(std::string& str, std::string split)
{
    std::vector<std::string> splits;
    const char* pSplitStart = split.c_str();
    const char* pSplit = strchr(split.c_str(), '|');
    while (pSplit) {
        splits.push_back(split.substr(pSplitStart-split.c_str(), pSplit - pSplitStart));
        pSplitStart = pSplit + 1;
        pSplit = strchr(pSplitStart, '|');
    }

    if (pSplitStart && strcmp(pSplitStart, "") != 0) 
        splits.push_back(pSplitStart);

    std::vector<std::string> lastVt;
    lastVt.push_back(str);
    for (unsigned int i=0; i<splits.size(); i++) {
        std::vector<std::string> vtTmp;
        for (unsigned int j = 0; j < lastVt.size(); j++) {
            std::vector<std::string> vt = std_string_splitString(lastVt[j], splits[i].c_str());
            for (unsigned int k = 0; k < vt.size(); k++) {
                vtTmp.push_back(vt[k]);
            }
        }
        
        lastVt = vtTmp;
    }
    return lastVt;
}

int std_string_indexOf(std::string& str, const char* math)
{
	const char* p = strstr(str.c_str(), math);
	if (p) {
		return p - str.c_str();
	}
	return -1;
}

bool std_string_startWith(std::string& str, const char* math)
{
    return str.find(math) == 0;
}

bool std_string_endWith(std::string& str, const char* math)
{
    return str.rfind(math) == (str.length() - strlen(math));
}

bool std_string_miniComp(std::string& s, const char* s2)
{
    int i=0;
    const char* s1 = s.c_str();
    while (s1[i] && s2[i] && s1[i] == s2[i]) {
        i++;
    }

    return !(s1[i] && s2[i]);
}

bool std_string_fixComp(std::string& s, const char* s2, int len)
{
    int i = 0;
    const char* s1 = s.c_str();
    while (i<len && s1[i] && s2[i] && s1[i] == s2[i]) {
        i++;
    }
    return i==len;
}

bool std_string_isnum(std::string& s)
{
    std::string::iterator it = s.begin();
    for ( ; it < s.end(); ++it)
    {
        if (*it<'0' || *it>'9')
            return false;
    }
    return true;
}

long std_string_hex2num(std::string& s)
{
    char *offset;
    if(s.length() > 2) {
        if(s[0] == '0' && s[1] == 'x') {
            return strtol(s.c_str(), &offset, 0);
        }
    }
    return strtol(s.c_str(), &offset, 16);
}

char* std_strchr(char* s, char chr, int& len)
{
    int i=0;
    while (i<(len-1) && s[i] && s[i] != chr) {
        i++;
    }
    
    if (s[i] == chr) return &s[i];
	if (s[i] == 0) len = i+1;
    return NULL;
}

std::string std_string_urlDecode(const std::string& str) 
{ 
    std::string result; 
    int hex = 0; 
    for (size_t i = 0; i < str.length(); ++i) { 
        switch (str[i]) { 
        case '+': {
            result += ' ';
            break;
        }
        case '%': {
            if (isxdigit(str[i + 1]) && isxdigit(str[i + 2])) {
                std::string hexStr = str.substr(i + 1, 2); 
                hex = strtol(hexStr.c_str(), 0, 16);
                //字母和数字[0-9a-zA-Z]、一些特殊符号[$-_.+!*'(),] 、以及某些保留字[$&+,/:;=?@] 

                //可以不经过编码直接用于URL 
                if (!((hex >= 48 && hex <= 57) || //0-9 
                (hex >=97 && hex <= 122) ||   //a-z 
                (hex >=65 && hex <= 90) ||    //A-Z 
                //一些特殊符号及保留字[$-_.+!*'(),]  [$&+,/:;=?@] 
                hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29 
                || hex == 0x2a || hex == 0x2b|| hex == 0x2c || hex == 0x2d || hex == 0x2e || hex == 0x2f 
                || hex == 0x3A || hex == 0x3B|| hex == 0x3D || hex == 0x3f || hex == 0x40 || hex == 0x5f 
                )) { 
                    result += char(hex);
                    i += 2; 
                } else 
                    result += '%'; 
            }else { 
                result += '%'; 
            } 
            break; 
        }
        default: {
            result += str[i]; 
            break; 
        }
        }
    } 
    return result; 
}

std::string wstring_to_string(const std::wstring& wstr) {
	try {
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(wstr);
	}
	catch (...) {
		return "err";
	}
}

std::wstring string_to_wstring(const std::string& input)
{
	try {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.from_bytes(input);
	} catch (...) {
		return L"err";
	}
}

}