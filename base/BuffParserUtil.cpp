#include "BuffParserUtil.h"
#include "CommonDefine.h"

BuffParserUtil::BuffParserUtil()
{
}

BuffParserUtil::~BuffParserUtil()
{

}


bool BuffParserUtil::getDataPacket(RingBuffer & buff, char * &data, int &len)
{
	unsigned int totalSize = buff.getUsedSize();
	if ( totalSize < BUFFER_HEAD_LEN )
		return false;

	char pHead[BUFFER_HEAD_LEN];
	buff.copy( pHead, BUFFER_HEAD_LEN, BUFFER_HEAD_LEN);
	unsigned int msglen = *(unsigned int*)(pHead);

	if ( totalSize < msglen + BUFFER_HEAD_LEN )
		return false;

	char * p = new char[msglen + 1];
	p[msglen] = 0;
	buff.remove(BUFFER_HEAD_LEN);
	buff.read(p, msglen, msglen);
	
	data = p;
	len = msglen;

	return true;
}

bool BuffParserUtil::writeDataPacket(RingBuffer & buff, char * &data, int &len)
{
	int size = buff.getFreeSize();
	if (size < BUFFER_HEAD_LEN + len )
		return false;
	len = (len);
	buff.write((char *)&len, BUFFER_HEAD_LEN);
	buff.write(data, len);
	return true;
}

