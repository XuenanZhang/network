#ifndef __FILEUTILS_H__
#define __FILEUTILS_H__

#include <vector>
#include "CommonDefine.h"

/************************************************************************
 文件操作方式为c的File*                          
 文件使用方式: 
				r(读), w(写), a(追加), t(文本，默认), b(二进制), +(读和写)
				 r和a开头文件必须存在，w不存在则创建                                                                      
 读写函数：                                                                      
          字符读写：fgetc和fputc	字符串读写：fgets和fputs 
			 数据块读写：fread和fwrite 格式化读写：fscanf和fprinf(磁盘）
 移动文件指引：            
          rewind（文件首）
			 fseek(file*, offset, startPos) startPos: SEEK_SET(首) SEEKCUR(当前) SEEK_END(尾)
************************************************************************/
namespace FileUtils
{
	std::string getCurrPath();

	void writeFile(const char * path, const char *info, bool isAppend = true, bool isEnter = true);
	void readFile(const char * path, char *info, int size);

	void getFiles(std::string path, std::vector<std::string>& files, const char * existStr = NULL);
}



#endif