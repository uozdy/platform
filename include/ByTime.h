#ifndef __PLATFORM_SRC_PLATFORM_TIME__
#define __PLATFORM_SRC_PLATFORM_TIME__

namespace PLATFORM
{
	class CTime
	{
	public:
		static unsigned long long GetUTCTime_Ms();//UTC time, 单位 ms
		static unsigned long long GetLocalTime_Ms();//UTC time, 单位 ms

		static unsigned int GetUTCTime_S(); //UTC time, 单位 s
		static unsigned int GetLocalTime_S(); //本地时区时间, 单位 s
		static unsigned int UTC2Localtime(unsigned int t);
		static unsigned int Localtime2UTC(unsigned int t);

		static unsigned int GetTimeZone();
	};
}

#endif
