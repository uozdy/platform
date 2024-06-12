#ifndef __PLATFORM_SRC_PLATFORM_TIME__
#define __PLATFORM_SRC_PLATFORM_TIME__

namespace PLATFORM
{
	class CTime
	{
	public:
		static unsigned long long GetUTCTime_Ms();//UTC time, ��λ ms
		static unsigned long long GetLocalTime_Ms();//UTC time, ��λ ms

		static unsigned int GetUTCTime_S(); //UTC time, ��λ s
		static unsigned int GetLocalTime_S(); //����ʱ��ʱ��, ��λ s
		static unsigned int UTC2Localtime(unsigned int t);
		static unsigned int Localtime2UTC(unsigned int t);

		static unsigned int GetTimeZone();
	};
}

#endif
