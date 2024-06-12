#include "IniFile.h"
#include <string.h>
#include <stdlib.h>
#include <chrono>

namespace PLATFORM
{
    CIniFile::CIniFile()
	{
	}
	
	CIniFile::~CIniFile()
	{
	}
	
	bool CIniFile::Load(const char* file)
	{
        m_strFile = file;
        
		FILE* fp = fopen(file, "rb");
		if (!fp) return false;
		
		#define LINELEN		1024
		#define SECTIONLEN	128
		#define KEYLEN 		128
		#define VALUELEN 	128
		char	line[LINELEN];
		char	section[SECTIONLEN];
		char	key[KEYLEN];
		char	value[VALUELEN];
		
		keyMap*	_keyMap = NULL;	
		while(!feof(fp)){
			if (fgets(line, LINELEN, fp)){
				if ( (1 == sscanf(line, "%*[^[][%128[^]^ ]", section)) ||  (1 == sscanf(line, "[%128[^]^ ]", section))){
					_keyMap = &m_iniMap[section];
				}else if ( _keyMap && (2 == sscanf(line, "%128s = %128[^;^\n^ ]", key, value) || 2 == sscanf(line, "%128[^=] = %128[^;^\n^ ]", key, value)) ){
					(*_keyMap)[key] = value;
				}
			}
		}
		fclose(fp);
		return true;
	}
	
	bool CIniFile::Save()
	{
		if (m_strFile.empty()) return false;
		
		char tmp[32];
		std::string tmpFile = m_strFile;
		
		auto t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		sprintf(tmp, "_%ld_bak", (long)t);

		tmpFile += tmp;
		
		FILE* fp = fopen(tmpFile.c_str(), "wb");
		if (!fp)	return false;

		std::string str;
		std::map<std::string, keyMap>::iterator it = m_iniMap.begin();
		for(; it != m_iniMap.end(); ++it){
			str = "[";
			str += it->first;
			str += "]\n";
			fwrite(str.c_str(), str.length(), 1, fp);
			keyMap::iterator pos = (it->second).begin();
			for(; pos != (it->second).end(); ++pos){
				str = pos->first;
				str += "= ";
				str += pos->second;
				str += ";\n";
				fwrite(str.c_str(), str.length(), 1, fp);
			}
			fwrite("\n", 1, 1, fp);
		}
		fclose(fp);
		
		rename(tmpFile.c_str(), m_strFile.c_str());
		return true;
	}
	
	bool CIniFile::GetValue(const char* sec, const char* key, char* value)
	{
		std::map<std::string, keyMap>::iterator it = m_iniMap.find(sec);
		if (it != m_iniMap.end()){
			keyMap::iterator pos = (it->second).find(key);
			if (pos != it->second.end()){
				strcpy(value, (it->second)[key].c_str());
				return true;
			}
		}
		value[0] = 0;
		return false;
	}
	
	bool CIniFile::SetValue(const char* sec, const char* key, const char* value)
	{
        if (!sec || !key || !value) return false;
        keyMap*	_keyMap = NULL;	
        _keyMap = &m_iniMap[sec];
        (*_keyMap)[key] = value;

		return true;
	}
	
	bool CIniFile::GetValue(const char* sec, const char* key, int& value)
	{
		std::map<std::string, keyMap>::iterator it = m_iniMap.find(sec);
		if (it != m_iniMap.end()){
			keyMap::iterator pos = (it->second).find(key);
			if (pos != it->second.end()){
				value = atoi((it->second)[key].c_str());
				return true;
			}
		}
		return false;
	}
	
	bool CIniFile::SetValue(const char* sec, const char* key, int value)
	{
        if (!sec || !key) return false;
        char tmp[10];
        keyMap*	_keyMap = NULL;	
        
        sprintf(tmp, "%d", value);
        _keyMap = &m_iniMap[sec];
        (*_keyMap)[key] = tmp;
        
		return true;
	}
	
	int  CIniFile::StartGetSection()
	{
		m_itMap = m_iniMap.begin();
		return 0;
	}
	
	bool CIniFile::GetNextSection(std::string& section)
	{
		if (m_itMap != m_iniMap.end()) {
			section = m_itMap->first;
			++m_itMap;
			return true;
		}
		return false;
	}
};
