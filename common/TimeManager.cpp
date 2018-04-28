#if defined(_WINDOWS)
#include <windows.h>
#include <time.h>
#elif defined(_LINUX)
#include <stdio.h>
#endif

#include "TimeManager.h"


static int leapyeardays[]		= {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int nonleapyeardays[]	= {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

TimeManager::TimeManager()
{

}

TimeManager::~TimeManager()
{
}


Bool TimeManager::init()
{
#if defined(_WINDOWS)
	_startTime = GetTickCount();
#elif defined(_LINUX)
	gettimeofday(&_tvStart , &_tz);
	_startTime = _tvStart.tv_sec*1000 + _tvStart.tv_usec/1000;
#endif
	_currTime = _startTime;

	time((time_t *)&_serverStartTime);
	setTime();

	return true;
}

UInt TimeManager::currentTime()
{
#if defined(_WINDOWS)
	_currTime = GetTickCount() - _startTime;
#elif defined(_LINUX)
	gettimeofday(&_tvEnd , &_tz);
	_currTime = _tvEnd.tv_sec*1000 + _tvEnd.tv_usec/1000 - _startTime;
#endif
	gettimeofday
	return _currTime;
}

void TimeManager::setTime()
{
	time((time_t *)&_setTime);
	setTMByANSITime(_setTime, _tm);
}

Time_t TimeManager::getANSITime()
{
	setTime();

	return _setTime;
}

TimeStruct TimeManager::getANSITimeStruct()
{
	setTime();
	return tm2ts(_tm);
}

void TimeManager::setTMByANSITime(Time_t &t, tm &tm)
{
#if defined(_WINDOWS)
	localtime_s(&tm, (time_t *)&t);
#elif defined(_LINUX)
	localtime_r(&t, &tm);
#endif
}

Time_t TimeManager::getANSITimeByTM(tm &tm)
{
	return (Time_t)mktime(&tm);
}

UInt TimeManager::time2uint(Time_t t)
{
	TimeStruct ts;
	if ( t == 0 )
		ts = TimeManager::Ptr()->getANSITimeStruct();
	else
		ts = sec2ts(t);
		

	UInt ret = 0;

	ret += ts.year - 2000;
	ret *= 100;

	ret += ts.month;
	ret *= 100;

	ret += ts.day;
	ret *= 100;

	ret += ts.hour;
	ret *= 100;

	ret += ts.minute;

	return ret;
}

Time_t TimeManager::uint2time(UInt t)
{
	TimeStruct ts = uint2timestruct(t);

	return ts2sec(ts);
}

TimeStruct TimeManager::uint2timestruct(UInt t)
{
	TimeStruct ts;
	ts.minute = t % 100;
	t /= 100;
	ts.hour = t % 100;
	t /= 100;
	ts.day = t % 100;
	t /= 100;
	ts.month = t % 100;
	t /= 100;
	ts.year = t % 100 + 2000;

	return ts;
}

tm TimeManager::ts2tm(TimeStruct &t)
{
	tm tm;
	tm.tm_year = t.year - 1900;
	tm.tm_mon = t.month - 1;
	tm.tm_mday = t.day;
	tm.tm_hour = t.hour;
	tm.tm_min = t.minute;
	tm.tm_sec = t.second;

	return tm;
}

TimeStruct TimeManager::tm2ts(tm &tm)
{
	TimeStruct ts;
	ts.year = tm.tm_year + 1900;
	ts.month = tm.tm_mon + 1;
	ts.day = tm.tm_mday;
	ts.hour = tm.tm_hour;
	ts.minute = tm.tm_min;
	ts.second = tm.tm_sec;
	ts.week = tm.tm_wday ? tm.tm_wday : 7;

	return ts;
}

UInt TimeManager::secToMonthDay(Time_t t)
{
	TimeStruct ts = sec2ts(t);

	if ( isLeapYear(ts.year) )
		return leapyeardays[ts.month - 1];
	else
		return nonleapyeardays[ts.month - 1];
}

TimeStruct TimeManager::sec2ts(Time_t t)
{
	tm tm;
	setTMByANSITime(t, tm);
	return tm2ts(tm);
}

UInt TimeManager::sec2week(Time_t t)
{
	tm tm;
	setTMByANSITime(t, tm);
	UInt week = tm2ts(tm).week;
	return week ? week : 7;
}

UInt TimeManager::ymd2week(UInt y, UInt m, UInt d)
{
	tm tm;
	tm.tm_year = y - 1900;
	tm.tm_mon = m - 1;
	tm.tm_mday = d;
	tm.tm_hour = 0;
	tm.tm_min = 0;
	tm.tm_sec = 0;
	return sec2week(getANSITimeByTM(tm));
}

Time_t TimeManager::ts2sec(TimeStruct &ts)
{
	tm tm = ts2tm(ts);
	return getANSITimeByTM(tm);
}

Time_t TimeManager::getTodayTime()
{
	Time_t t = (Time_t)time(NULL);
	tm tm;
	setTMByANSITime(t, tm);
	TimeStruct ts;
	tm.tm_hour = 0;
	tm.tm_min = 0;
	tm.tm_sec = 0;
	return getANSITimeByTM(tm);
}

Time_t TimeManager::getWeekTime()
{
	UInt t = getTodayTime();
	TimeManager::Ptr()->setTime();
	UInt week = TimeManager::Ptr()->getWeek();

	return t - 86400*(week - 1);
}

Time_t TimeManager::getMonthTime()
{
	Time_t t = getTodayTime();
	TimeManager::Ptr()->setTime();
	UInt day = TimeManager::Ptr()->getDay();

	return t - 86400*(day - 1);
}

Bool TimeManager::isSecInToday(Time_t t)
{
	Time_t todayTime = getTodayTime();
	if ( t >= todayTime && t < todayTime + 86400 )
		return true;

	return false;
}

Bool TimeManager::isSecInWeek(Time_t t)
{
	Time_t weekTime = getWeekTime();
	if ( t >= weekTime && t < weekTime + 7*86400 )
		return true;

	return false;
}

Bool TimeManager::isSecInMonth(Time_t t)
{
	Time_t monthTime = getMonthTime();
	Int day = secToMonthDay(TimeManager::Ptr()->getANSITime());
	
	if ( t >= monthTime && t < monthTime + day*86400 )
		return true;

	return false;
}
