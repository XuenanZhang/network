//#include <stdlib.h>
#include <stdio.h>
#include "FileUtils.h"

#ifdef _WINDOWS
#include <Windows.h>
#include <io.h>
#pragma warning(disable:4996)
#elif defined(_LINUX)
#endif

using namespace std;
namespace FileUtils
{
	string getCurrPath()
	{
		char szPath[256];
#ifdef _WINDOWS
		::GetModuleFileNameA(NULL, szPath, MAX_PATH);
		strrchr((char *)szPath, '\\')[1] = 0;
#endif 

		return szPath;
	}

	void writeFile(const char * path, const char *info, bool isAppend, bool isEnter)
	{
		if (path == NULL || info == NULL)
			return;

		FILE * fp = fopen(path, "a");
		if (fp == NULL)
		{
			if (isAppend)
				fp = fopen(path, "w");
			
			if (fp == NULL)
			{
				printf("open file error path = %s", path);
				return;
			}
		}
		else
		{
			if (isEnter)
				fputc('\n', fp);
		}

		fputs(info, fp);
		if ( 0 !=fclose(fp) )
			printf("close file error path = %s", path);
	}

	void readFile(const char * path, char *info, int size)
	{
		if (path == NULL || info == NULL)
			return;

		FILE * fp = fopen(path, "r+");
		if (fp == NULL)
		{
			printf("open file error path = %s", path);
			return;
		}

		char * buff = info;
		int temp = 0;
		while (fgets(buff, size, fp))
		{
			temp = strlen(buff);
			buff += temp;
			size -= temp;
		}
		printf(info);
		if ( 0 !=fclose(fp) )
			printf("close file error path = %s", path);
	}

	void getFiles(string path, vector<string>& files, const char * existStr)
	{
#ifdef _WINDOWS
		//文件句柄  
		long   hFile = 0;
		//文件信息  
		struct _finddata_t fileinfo;
		string p;
		if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
		{
			do
			{
				//如果是目录,迭代之  
				//如果不是,加入列表  
				if ((fileinfo.attrib &  _A_SUBDIR))
				{
					if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
						getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
				else
				{
					p.assign(path).append("\\").append(fileinfo.name);
					if (existStr == NULL || p.find(existStr) != string::npos)
						files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				}
			} while (_findnext(hFile, &fileinfo) == 0);
			_findclose(hFile);
		}
#endif
	}
}
