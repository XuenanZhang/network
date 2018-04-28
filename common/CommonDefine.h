#ifndef __COMMONAPI_H__
#define __COMMONAPI_H__


#ifdef _WINDOWS
#include <string>
#pragma warning ( disable : 4996 )
#elif defined(_LINUX)
#include <string.h>
#endif 
#include "CharUtils.h"

typedef std::string String;
typedef std::wstring WString;
typedef bool Bool;
typedef char Char;
typedef short Short;
typedef int Int;
typedef long Long;
typedef long long LLong;
typedef unsigned char UChar;
typedef unsigned short UShort;
typedef unsigned int UInt;
typedef unsigned long ULong;
typedef unsigned long long ULLong;
typedef float Float;
typedef double Double;
typedef UInt Time_t;

//#if defined(_LINUX)
//#define NULL 0;
//#endif

//无效的句柄
#define INVALID_HANDLE	-1

//无效的ID值
#define INVALID_ID		-1

//真
#ifndef TRUE
	#define TRUE 1
#endif
//假
#ifndef FALSE
	#define FALSE 0
#endif

#ifndef NULL
	#define NULL 0
#endif



#define FRAME_RATE 50

#define SAFE_RET(a, b) do { if(a == b) return; } while(0);
#define SAFE_RET_VAL(a, b, ret) do { if(a == b) return ret; } while(0);
#define SAFE_CONTINUE(a, b) if(a == b) continue;
#define SAFE_BREAK(a, b) if(a == b) break;
#define SAFE_DEL(p)           do { delete (p); (p) = NULL; } while(0);
#define SAFE_DEL_ARR(p)     do { if(p) { delete[] (p); (p) = NULL; } } while(0);
#define SAFE_DEL_HANDLE(x) do { if(x!=NULL && x != INVALID_HANDLE_VALUE){CloseHandle(x);x = INVALID_HANDLE_VALUE;} } while(0);

#define Float_Zero_Value 0.000001f
#define Float_Is_Zero(f) fabs(f) < Float_Zero_Value
#define Float_Is_Equal(a, b) fabs((a) - (b)) < Float_Zero_Value

//字符串是否相等，区分大小写
#define STRING_IS_EQUAL(str1, str2) (CharUtils::strcmpA(str1, str2) == 0)
//字符串是否相等，不区分大小写
#define STRING_NOCASE_EQUAL(str1, str2) (CharUtils::strcmpnA(str1, str2) == 0)


//判断某位是否被置
//15.14....3.2.1.0 
#define ISSET0(x) ((x)&0x1)
#define ISSET1(x) ((x)&0x2)
#define ISSET2(x) ((x)&0x4)
#define ISSET3(x) ((x)&0x8)
#define ISSET4(x) ((x)&0x10)
#define ISSET5(x) ((x)&0x20)
#define ISSET6(x) ((x)&0x40)
#define ISSET7(x) ((x)&0x80)
#define ISSET8(x) ((x)&0x100)
#define ISSET9(x) ((x)&0x200)
#define ISSET10(x) ((x)&0x400)
#define ISSET11(x) ((x)&0x800)
#define ISSET12(x) ((x)&0x1000)
#define ISSET13(x) ((x)&0x2000)
#define ISSET14(x) ((x)&0x4000)
#define ISSET15(x) ((x)&0x8000)

#endif

