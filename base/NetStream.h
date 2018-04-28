/************************************************************************/
/*								网络数据包流化							*/
/************************************************************************/
#ifndef _NETSTREAM_H_
#define _NETSTREAM_H_

#include "CommonDefine.h"
#include "NetDefine.h"
class NetStream
{
public:
	NetStream();
	~NetStream();

public:
	NetStream & operator << (Char &val);
	NetStream & operator << (Short &val);
	NetStream & operator << (Int &val);
	NetStream & operator << (LLong &val);

	NetStream & operator << (UChar &val);
	NetStream & operator << (UShort &val);
	NetStream & operator << (UInt &val);
	NetStream & operator << (ULLong &val);

	NetStream & operator << (Float &val);
	NetStream & operator << (Double &val);

	NetStream & operator << (const String &val);
	NetStream & addData(const void *pData, unsigned int len );
public:
	NetStream & operator >> (Char &val);
	NetStream & operator >> (Short &val);
	NetStream & operator >> (Int &val);
	NetStream & operator >> (LLong &val);

	NetStream & operator >> (UChar &val);
	NetStream & operator >> (UShort &val);
	NetStream & operator >> (UInt &val);
	NetStream & operator >> (ULLong &val);

	NetStream & operator >> (Float &val);
	NetStream & operator >> (Double &val);

	NetStream & operator >> (String &val);
	NetStream & outputData(void *pData, unsigned int len );

public:
	char * getBuff();
	Int getSize();
	void setBuffData(const void *pData, unsigned int len );
	void reset(){_currPos = 0;};
private:
	Int _currPos;
	char _buff[MAX_PACKET_SIZE];
};



#endif
