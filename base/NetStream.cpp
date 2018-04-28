#include "NetStream.h"

#define CheckRange(val) if( _currPos + (val) > MAX_PACKET_SIZE) { printf("error NetStream is full") ;return *this;}

NetStream::NetStream()
{
	_currPos = 0;
	memset(_buff,0,MAX_PACKET_SIZE);
}

NetStream::~NetStream()
{
}

char * NetStream::getBuff()
{
	return _buff;
}

Int NetStream::getSize()
{
	return _currPos;
}	

void NetStream::setBuffData(const void *pData, unsigned int len )
{
	addData(pData, len);
	_currPos = 0;
}

NetStream & NetStream::operator << (Char &val)
{
	return addData(&val, sizeof(Char));
}

NetStream & NetStream::operator << (Short &val)
{
	return addData(&val, sizeof(Short));
}

NetStream & NetStream::operator << (Int &val)
{
	return addData(&val, sizeof(Int));
}

NetStream & NetStream::operator << (LLong &val)
{
	return addData(&val, sizeof(LLong));
}

NetStream & NetStream::operator << (UChar &val)
{
	return addData(&val, sizeof(UChar));
}
NetStream & NetStream::operator << (UShort &val)
{
	return addData(&val, sizeof(UShort));
}

NetStream & NetStream::operator << (UInt &val)
{
	return addData(&val, sizeof(UInt));
}

NetStream & NetStream::operator << (ULLong &val)
{
	return addData(&val, sizeof(ULLong));
}

NetStream & NetStream::operator << (Float &val)
{
	return addData(&val, sizeof(Float));
}

NetStream & NetStream::operator << (Double &val)
{
	return addData(&val, sizeof(Double));
}

NetStream & NetStream::operator << (const String &val)
{
	UShort len = val.length();
	addData(&len, sizeof(UShort));
	return addData(val.c_str(), len);
}

NetStream & NetStream::addData(const void * pData, unsigned int len )
{
	CheckRange(len);
	memcpy(_buff + _currPos, pData, len);
	_currPos += len;
	return *this;
}	

NetStream & NetStream::operator >> (Char &val)
{
	CheckRange(sizeof(Char));
	val = *(Char*)&_buff[_currPos];
	_currPos += sizeof(Char);
	return *this;
}

NetStream & NetStream::operator >> (Short &val)
{
	CheckRange(sizeof(Short));
	val = *(Short*)&_buff[_currPos];
	_currPos += sizeof(Short);
	return *this;
}

NetStream & NetStream::operator >> (Int &val)
{
	CheckRange(sizeof(Int));
	val = *(Int*)&_buff[_currPos];
	_currPos += sizeof(Int);
	return *this;
}

NetStream & NetStream::operator >> (LLong &val)
{
	CheckRange(sizeof(LLong));
	val = *(LLong*)&_buff[_currPos];
	_currPos += sizeof(LLong);
	return *this;
}

NetStream & NetStream::operator >> (UChar &val)
{
	CheckRange(sizeof(UChar));
	val = *(UChar*)&_buff[_currPos];
	_currPos += sizeof(UChar);
	return *this;
}
NetStream & NetStream::operator >> (UShort &val)
{
	CheckRange(sizeof(UShort));
	val = *(UShort*)&_buff[_currPos];
	_currPos += sizeof(UShort);
	return *this;
}

NetStream & NetStream::operator >> (UInt &val)
{
	CheckRange(sizeof(UInt));
	val = *(UInt*)&_buff[_currPos];
	_currPos += sizeof(UInt);
	return *this;
}

NetStream & NetStream::operator >> (ULLong &val)
{
	CheckRange(sizeof(ULLong));
	val = *(ULLong*)&_buff[_currPos];
	_currPos += sizeof(ULLong);
	return *this;
}

NetStream & NetStream::operator >> (Float &val)
{
	CheckRange(sizeof(Float));
	val = *(Float*)&_buff[_currPos];
	_currPos += sizeof(Float);
	return *this;
}

NetStream & NetStream::operator >> (Double &val)
{
	CheckRange(sizeof(Double));
	val = *(Double*)&_buff[_currPos];
	_currPos += sizeof(Double);
	return *this;
}

NetStream & NetStream::operator >> (String &val)
{
	CheckRange(sizeof(UShort));
	UShort strLen = *(UShort*)&_buff[_currPos];
	CheckRange(strLen + sizeof(UShort));
	_currPos += sizeof(UShort);
	char temp[MAX_PACKET_SIZE] = "";
	memcpy(temp, &_buff[_currPos], strLen);
	val = temp;
	return *this;
}

NetStream & NetStream::outputData( void * pData, unsigned int len )
{
	CheckRange(len);
	memcpy(pData, &_buff[_currPos], len);
	_currPos += len;
	return *this;
}	