#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <vector>
#include "CharUtils.h"

namespace CharUtils
{
	int strToInt(const char * value)
	{
		if (value == NULL)
			return 0;

		return atoi(value);
		/*
		if (value == NULL)
			return 0;
		int sign = 1;
		int i = 0;
		int t = 0;
		int tt = 0;

		if (value[0] == '-')
		{
			sign = -1;
			++i;
		}

		while (value[i])
		{
			if (value[i] >= '0' && value[i] <= '9')
			{
				tt = value[i] - '0';
				t *= 10;
				t += tt;
			}
			else
				break;
			++i;
		}
		return t*sign;
		*/
	}

	int strToInt(const string & value)
	{
		return strToInt(value.c_str());
	}

	unsigned int strToUInt(const char * value, int radix)
	{
		return (unsigned int)strToULLong(value, radix);
	}

	unsigned int strToUInt(const string & value, int radix)
	{
		return strToUInt(value.c_str(), radix);
	}

	long long strToLLong(const char * value)
	{
#ifdef _WINDOWS
		return _atoi64( value );
#elif defined(_LINUX)
		return atoll( value );
#endif
		return 0;
	}

	long long strToLLong(const string & value)
	{
		return strToLLong(value.c_str());
	}

	unsigned long long strToULLong(const char * value, int radix)
	{
#ifdef _WINDOWS
		return _strtoui64( value , NULL , radix );
#elif defined(_LINUX)
		return strtoull( value , NULL , radix  );
#endif
		return 0;
	}

	unsigned long long strToULLong(const string & value, int radix)
	{
		return strToULLong(value.c_str());
	}

	float strToFloat(const char * value)
	{
		if(value == NULL)
			return 0.0f;

		return (float)atof(value);
	}

	float strToFloat(const string & value)
	{
		return strToFloat(value.c_str());
	}

	double strToDouble(const char * value)
	{
		if(value == NULL)
			return 0.0;

		return atof(value);
	}

	double strToDouble(const string & value)
	{
		return strToDouble(value.c_str());
	}

	string intToStr(int val)
	{
		char buf[32] = "";
		sprintfA(buf, 32, "%d", val);
		return buf;
	}

	string uintToStr(unsigned int val)
	{
		char buf[32] = "";
		sprintfA(buf, 32, "%d", val);
		return buf;
	}

	string floatToStr(float val)
	{
		char buf[32] = "";
		sprintfA(buf, 32, "%f", val);
		return buf;
	}

	string uLLongToStr(unsigned long long val)
	{
		char buf[32] = "";
		sprintfA(buf, 32, "%llu" , val );
		return buf;
	}


	void sprintfA(char *buffer, int size, const char *format, ...)
	{
		va_list params;
		va_start(params, format);
		vsnprintf(buffer, size, format, params);
		va_end(params);
	}

	void sprintfAW( wchar_t *buffer , int size , const wchar_t *format , ... )
	{
		va_list pArgList;
		va_start(pArgList, format);
		vswprintf(buffer, size, format, pArgList);
		va_end(pArgList);
	}

/*	string format(const char* format, ...)
	{
#define MY_MAX_STRING_LENGTH (512)

		string ret;

		va_list params;
		va_start(params, format);

		char* buf = (char*)malloc(MY_MAX_STRING_LENGTH);
		//memset(buf, 0, sizeof(buf));

		if (buf != NULL)
		{
			vsnprintf(buf, MY_MAX_STRING_LENGTH, format, params);
			ret = buf;
			free(buf);
		}
		va_end(params);

		return ret;
	}
*/

	string format( const char *format, ... )
	{
		string strResult = "";
		if (NULL != format)
		{
			va_list params;
			va_start(params, format);                           
#ifdef _WINDOWS
			size_t len = _vscprintf(format, params) + 1;
#elif defined(_LINUX)
			size_t len = vsnprintf(NULL,0,format,params) + 1;
			va_end(params);                                    //重置变量参数
			va_start(params, format);                            //初始化变量参数
#endif
			vector<char> vBuffer(len, '\0');    //创建用于存储格式化字符串的字符数组
			vsnprintf(&vBuffer[0], len, format, params);
			strResult = &vBuffer[0];
			va_end(params);                                    //重置变量参数
		}
		return strResult;
	}

	int strcmpA(const char * p1, const char * p2)
	{
		for (int i = 0;;++i)
		{
			if ( p1[i] < p2[i] ) 
				return -1;
			else if ( p1[i] > p2[i] ) 
				return 1;
			
			if (p1[i] == 0)
				break;
		}

		return 0;
	}

	int strcmpnA(const char * p1, const char * p2)
	{
		for (int i = 0;;++i)
		{
			char a = p1[i];
			if ( a >= 'a'  && a <= 'z' ) a = a & 0xDF;

			char b = p1[i];
			if ( b >= 'a'  && b <= 'z' ) b = b & 0xDF;

			if ( a < b )
				return -1;
			else if ( a > b )
				return 1;

			if ( a == 0 )
				break;
		}

		return 0;
	}
}
