/************************************************************************/
/*							一次读取一个包								*/
/************************************************************************/
#ifndef _BUFFPARSERPACK_H_
#define _BUFFPARSERPACK_H_

#include "CommonDefine.h"
#include "PoolObj.h"
#include "BuffParser.h"

class ListenerLibEvent;
struct bufferevent;
class BuffParserPack : public BuffParser
{
public:
	BuffParserPack(){};
	virtual ~BuffParserPack(){};

public:
	virtual void onRead();
	virtual bool sendData(char * data, int len);

	char * getData() {return _data; };
	int getTotal(){return _total; };
protected:
	virtual void clear();

private:
	int _total;
	char * _data;
	
};

#endif