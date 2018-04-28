/************************************************************************/
/*						推送模式（直接转发bufferevent中的数据	）			*/
/************************************************************************/
#ifndef _BUFFPARSER_H_
#define _BUFFPARSER_H_
#include "CommonDefine.h"
class bufferevent;
class BuffParserPush
{
public:
	BuffParserPush(bufferevent *pBev, Int netId);
	virtual ~BuffParserPush();

public:
	virtual bool onRead();

	virtual bool sendData(char * data, int len);

private:
};

#endif