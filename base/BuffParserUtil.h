/************************************************************************/
/*							处理数据收发解析器							*/
/************************************************************************/
#ifndef _BuffParserUtil_H_
#define _BuffParserUtil_H_

#include "NetDefine.h"
#include "RingBuffer.h"


class BuffParserUtil
{
public:
	BuffParserUtil();
	virtual ~BuffParserUtil();

public:
	static bool getDataPacket( RingBuffer & buff, char * &data, int &len);
	static bool writeDataPacket(RingBuffer & buff, char * &data, int &len);

};

#endif