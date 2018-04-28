/****************************************************************************/
/*		    字符串数字转换,格式化字符串，编码集转换工具类                     */
/****************************************************************************/

#ifndef __CHAR_UTILS_H__
#define __CHAR_UTILS_H__
#include <string>
#include "CommonDefine.h"
using namespace std;

namespace CharUtils
{
	/** 字符串转换为数字 **/
	int strToInt(const char * value);
	int strToInt(const string & value);
	unsigned int strToUInt(const char * value, int radix = 10);
	unsigned int strToUInt(const string & value, int radix = 10);
	long long strToLLong(const char * value);
	long long strToLLong(const string & value);
	unsigned long long strToULLong(const char * value, int radix = 10);
	unsigned long long strToULLong(const string & value, int radix = 10);
	float strToFloat(const char * value);
	float strToFloat(const string & value);
	double strToDouble(const char * value);
	double strToDouble(const string & value);

	/** 数字转换为字符串 **/
	string intToStr(int val);
	string uintToStr(unsigned int val);
	string floatToStr(float val);
	string uLLongToStr(unsigned long long val);

	/** 字符串格式化 **/
	void sprintfA(char *buffer, int size, const char *format, ...);
	void sprintfAW( wchar_t *buffer , int size , const wchar_t *format , ... );
	string format(const char* format, ...);

	/** 字符串比较 **/
	//区分大小写
	int strcmpA(const char * p1, const char * p2);
	//不区分大小写
	int strcmpnA(const char * p1, const char * p2);
}

#endif