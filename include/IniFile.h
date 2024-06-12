#ifndef _INIFILE_H__
#define _INIFILE_H__
#include <stdio.h>
#include <map>
#include <string>
namespace PLATFORM
{
	typedef std::map<std::string, std::string>			keyMap;
	typedef std::map<std::string, keyMap>::iterator		IniFileIter;
    class CIniFile
	{
	public:
		CIniFile();
		~CIniFile();
	public:
		bool Load(const char* file);
		bool Save();
		bool GetValue(const char* sec, const char* key, char* value);
		bool SetValue(const char* sec, const char* key, const char* value);
		
		bool GetValue(const char* sec, const char* key, int& value);
		bool SetValue(const char* sec, const char* key, int value);
	public:
		int  StartGetSection();
		bool GetNextSection(std::string& section);
		
	private:		
		std::map<std::string, keyMap>				m_iniMap;
		std::string									m_strFile;
		IniFileIter									m_itMap;
	};
}
#endif
