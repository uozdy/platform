#ifndef __STD_STRING_EXT_H__
#define __STD_STRING_EXT_H__
#include <string>
#include <vector>
#include <codecvt>
#include <locale>
namespace PLATFORM
{
	
std::string & std_string_format(std::string & _str, const char * _Format, ...);
std::string std_string_intToString(int i);
std::string std_string_format(const char * _Format, ...);
std::vector<std::string> std_string_splitString(std::string& str, const char* split);
std::vector<std::string> std_string_splitsString(std::string& str, std::string split);
int  std_string_indexOf(std::string& str, const char* math);
bool std_string_startWith(std::string& str, const char* math);
bool std_string_endWith(std::string& str, const char* math);
bool std_string_miniComp(std::string& s1, const char* s2);
bool std_string_fixComp(std::string& s1, const char* s2, int len);
bool std_string_isnum(std::string& s);
long std_string_hex2num(std::string& s);

char* std_strchr(char* s, char chr, int& len);

std::string std_string_urlDecode(const std::string& str);

std::string wstring_to_string(const std::wstring& wstr);
std::wstring string_to_wstring(const std::string& input);

}
#endif