#ifndef __LOG_H__
#define __LOG_H__
#include <string>
#include "Singleton.h"

using namespace std;
enum LogType
{
	LogType_Normal,
	LogType_Warning,
	LogType_Error,
	LogType_Max,
};


class Log : EXTEND_SINGLETON(Log)
{
public:
	Log(){};
	~Log(){};

public:
	/**
	 * ¥Ú”°»’÷æ
	 */
	void printfLog(LogType type, const char * pStr, ...);
};

#endif