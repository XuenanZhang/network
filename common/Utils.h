#ifndef __UTILS_H__
#define __UTILS_H__

#include <vector>
#include <string>
#include <math.h>
#include "CommonDefine.h"


namespace Utils
{
	void sleep( UInt millionseconds, UInt sleepLimits = FRAME_RATE);

	template< typename T >
	void swap(T &val1, T &val2)
	{
		T temp = val1;
		val1 = val2;
		val2 = temp;
	}

	/** 随机 **/
	int random(int min, int max);

	/** 随机多个不重复的项 **/
	template<typename T>
	void randomMultiByVec(std::vector<T> & srcVec, int num, std::vector<T> & targetVec)
	{
		if (srcVec.empty() )
			return;
		vector<T> vec(srcVec.begin(), srcVec.end());
		do
		{
			size_t size = vec.size();
			int n = random(0, size - 1);
			targetVec.push_back(vec[n]);
			vec.erase(vec.begin() + n);
		} while (--num && !vec.empty());
	}
}


namespace StrUtils
{
	

	/* 拆分字符串 */
	bool splitString(const std::string & str, std::vector<std::string> & reVec, const std::string & delimit);
	bool splitString(const std::string & str, std::vector<int> & reVec, const std::string & delimit);

	/* 合并字符串 */
	bool mergeString(const std::vector<std::string> & vec, std::string & reStr, const std::string & delimit);
	bool mergeString(const std::vector<int> & vec, std::string & reStr, const std::string & delimit);
}

#endif