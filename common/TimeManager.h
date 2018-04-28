#ifndef __TIMEMANAGER_H__
#define __TIMEMANAGER_H__

#include "CommonDefine.h"
#include "Singleton.h"
#ifdef __LINUX__
#include <sys/utsname.h>
#include <sys/time.h>
#endif

struct TimeStruct
{
	UInt year;
	UInt month;
	UInt day;
	UInt hour;
	UInt minute;
	UInt second;
	UInt week;

	TimeStruct()
	{
		year = 0;
		month = 0;
		day = 0;
		hour = 0;
		minute = 0;
		second = 0;
		week = 0;
	};
};

class TimeManager : EXTEND_SINGLETON(TimeManager)
{
public:
	TimeManager();
	~TimeManager();

public:
	Bool init();

	/** 计算一次当前运行时间，并返回（毫秒） **/
	UInt currentTime();

	/** 直接返回上次计算的当前时间 **/
	UInt currentSavedTime() { return _currTime; };

	/** 服务器启动时间 (距离1970.1.1 秒数） **/
	UInt serverStartTime(){ return _serverStartTime; };

	/** 将当前的系统时间格式化到时间管理器里 **/
	void setTime();

	/** 得到标准时间 (距离1970.1.1 秒数） **/
	Time_t getANSITime( );

	/** 得到标准时间TimeStruct **/
	TimeStruct getANSITimeStruct();

public:
	/** 取得设置时间时候的“年、月、日、时、分、秒、星期的值” **/
	UInt getYear() { return _tm.tm_year + 1900; }	//[1900, ~]
	UInt getMonth() { return _tm.tm_mon + 1; }		//[1, 12]
	UInt getDay() { return _tm.tm_mday; }			//[1, 31]
	UInt getHour() { return _tm.tm_hour; }			//[0, 23]
	UInt getMinute() { return _tm.tm_min; }			//[0, 59]
	UInt getSecond() { return _tm.tm_sec; }			//[0, 59]
	UInt getWeek() { return _tm.tm_wday ? _tm.tm_wday : 7; }	//[1, 7]

public:
	/** 是否闰年 **/
	static Bool isLeapYear( UInt year ) { return (year % 400 == 0) ?  true : ((year % 4 == 0) && (year % 100 != 0)); }
	/** 某个时间当月天数 **/
	static UInt secToMonthDay(Time_t t);

	/** 时间戳转日期 **/
	static TimeStruct sec2ts(Time_t t);
	/** 时间戳转星期几 **/
	static UInt sec2week(Time_t t);
	/** 年月日转星期几 **/
	static UInt ymd2week(UInt y, UInt m, UInt d);
	/** 日期转时间戳 **/
	static Time_t ts2sec(TimeStruct &ts);
	
	/** 获得今天开始秒数 **/
	static Time_t getTodayTime();
	/** 获得这个星期开始秒数 **/
	static Time_t getWeekTime();
	/** 获得这个月开始秒数 **/
	static Time_t getMonthTime();

	/** 时间戳是否是在今天 **/
	static Bool isSecInToday(Time_t t);
	/** 时间戳是否是在本周 **/
	static Bool isSecInWeek(Time_t t);
	/** 时间戳是否是在本月 **/
	static Bool isSecInMonth(Time_t t);

	/** 将时间（年、月、日、小时、分）转换成一个UINT来表示,默认参数为当前时间
		例如：0,507,211,233 表示 "2005.07.21 12:33" **/
	static UInt time2uint(Time_t t = 0);
	static Time_t uint2time(UInt t);
	static TimeStruct uint2timestruct(UInt t);

private:
	static void setTMByANSITime(Time_t &t, tm &tm);
	static Time_t getANSITimeByTM(tm &tm);
	static tm ts2tm(TimeStruct &t);
	static TimeStruct tm2ts(tm &tm);
	
private:
	UInt	_startTime ;
	UInt	_serverStartTime;//起服时间 距离1970.1.1
	UInt	_currTime ;
	Time_t	_setTime ;
	tm		_tm ;
#ifdef __LINUX__
	struct timeval _tvStart, _tvEnd;
	struct timezone _tz;
#endif
};









#endif