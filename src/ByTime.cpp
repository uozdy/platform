#include "ByTime.h"
#include <stdio.h>
#include <time.h>
#include "platform.h"
namespace PLATFORM
{

	unsigned long long CTime::GetUTCTime_Ms()
	{
		long long msec = 0;
		struct timeval tv;
		gettimeofday( &tv, NULL );
		msec += (long long)tv.tv_sec * 1000ULL;
		msec += (long long)tv.tv_usec / 1000;
		return msec;
	}

	unsigned long long CTime::GetLocalTime_Ms()
	{
		long long msec = 0;
		struct timeval tv;
		struct timezone tz;
		gettimeofday( &tv, &tz );
		msec += (long long)tv.tv_sec * 1000ULL;
		msec += (long long)tv.tv_usec / 1000;
		return msec + tz.tz_minuteswest * 60 * 1000;
	}

	unsigned int CTime::GetUTCTime_S()
	{
		return time(0);
	}

	unsigned int CTime::GetLocalTime_S()
	{
		struct timeval tv;
		struct timezone tz;
		gettimeofday( &tv, &tz );
		return (unsigned int)(tv.tv_sec + tz.tz_minuteswest * 60);
	}

	unsigned int CTime::UTC2Localtime(unsigned int t)
	{
		struct timeval tv;
		struct timezone tz;
		gettimeofday( &tv, &tz );
		return (unsigned int)(t + tz.tz_minuteswest * 60);
	}

	unsigned int CTime::Localtime2UTC(unsigned int t)
	{
		struct timeval tv;
		struct timezone tz;
		gettimeofday( &tv, &tz );
		return (unsigned int)(t - tz.tz_minuteswest * 60);
	}

	unsigned int CTime::GetTimeZone()
	{
		struct timeval tv;
		struct timezone tz;
		gettimeofday( &tv, &tz );
		return tz.tz_minuteswest * 60;
	}
}



