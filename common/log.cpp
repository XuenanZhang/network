#include <stdarg.h>
#include "FileUtils.h"
#ifdef _MFCPRO
#include <afx.h>
#endif
#include "log.h"
#include "CharUtils.h"
#ifdef _LINUX
#include <stdio.h>
#endif



static const string LogTypeToName[LogType_Max] =
{
	"[ Normal ] ",
	"[ Warning ] ",
	"[ Error ] ",
};

void Log::printfLog(LogType type, const char * format, ...)
{
	char buff[256] = "";
	memset(buff, 0 , 256);
	va_list params;
	va_start(params, format);
#if defined(_WINDOWS)
	vsnprintf_s(buff, 256, format, params);
#elif defined(_LINUX)
	vsnprintf(buff, 256, format, params);
#endif
	va_end(params);
	string str = "";
	str += LogTypeToName[type];
	str += buff;
	str += "\n";
	
#ifdef _MFCPRO
	TRACE(str.c_str());
#else
	printf(str.c_str());
#endif
	if (type == LogType_Error || type == LogType_Warning)
	{
		FileUtils::writeFile("./errorLog", str.c_str());
	}
}