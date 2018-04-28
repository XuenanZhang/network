#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "Utils.h"
#include "CharUtils.h"

#ifdef _WINDOWS
#include <Windows.h>
#include <io.h>
#pragma warning(disable:4996)
#elif defined(_LINUX)
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#endif

namespace Utils
{
	void sleep( UInt millionseconds, UInt sleepLimits )
	{
		if ( millionseconds < sleepLimits )
			millionseconds = sleepLimits;

#if defined(_WINDOWS)
		::Sleep( millionseconds );
#elif defined(_LINUX)
		usleep(millionseconds * 1000);
#endif
	}

	int random(int min, int max)
	{
		if (min == max)
			return min;

		if (min > max)
		{
			swap(min, max);
		}

		int diff = max - min;
		if (diff < RAND_MAX)
			return ((rand()) % (diff + 1) + min);
		else
			return (static_cast<int>(static_cast<double>(rand()) / RAND_MAX * static_cast<double>(diff)) + min);
	}
};

namespace StrUtils
{
	
	bool splitString(const std::string & str, std::vector<std::string> & reVec, const std::string & delimit)
	{
		reVec.clear();
		if (str.empty())
			return false;
		int len = delimit.length();
		for (size_t last = 0, index = std::string::npos;;)
		{
			index = str.find_first_of(delimit, last);
			if (index == std::string::npos)
			{
				reVec.push_back(str.substr(last, std::string::npos));
				break;
			}
			else
			{
				reVec.push_back(str.substr(last, index - last));
				last = index + len;
			}
		}
		return true;
	}

	bool splitString(const std::string & str, std::vector<int> & reVec, const std::string & delimit)
	{
		reVec.clear();
		if (str.empty())
			return false;
		int len = delimit.length();
		for (size_t last = 0, index = std::string::npos;;)
		{
			index = str.find_first_of(delimit, last);
			if (index == std::string::npos)
			{
				reVec.push_back(CharUtils::strToInt(str.substr(last, std::string::npos)));
				break;
			}
			else
			{
				reVec.push_back(CharUtils::strToInt(str.substr(last, index - last)));
				last = index + len;
			}
		}
		return true;
	}

	bool mergeString(const std::vector<std::string> & vec, std::string & reStr, const std::string & delimit)
	{
		reStr = "";

		if (vec.empty())
			return  false;

		for (std::vector<std::string>::const_iterator iter = vec.begin(); iter != vec.end(); ++iter)
		{
			if (!reStr.empty())
				reStr += delimit;
			reStr += (*iter);
		}
		return true;
	}

	bool mergeString(const std::vector<int> & vec, std::string & reStr, const std::string & delimit)
	{
		reStr = "";

		if (vec.empty())
			return  false;

		char buff[20] = "";
		for (std::vector<int>::const_iterator iter = vec.begin(); iter != vec.end(); ++iter)
		{
			memset(buff, 0, sizeof(buff));
			CharUtils::sprintfA(buff, sizeof(buff), "%d", *iter); 
			if (!reStr.empty())
				reStr += delimit;
			reStr += buff;
		}

		return true;
	}
}



