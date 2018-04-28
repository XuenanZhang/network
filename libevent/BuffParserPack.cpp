#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include "BuffParserPack.h"
#include "NetDefine.h"

inline static void parserHead(char *buff, unsigned int *data)
{
	char *p = (char *)data;
	int len = BUFFER_HEAD_LEN;
	while (len--) *(p++) = *(buff++);
}

inline static int newBuff(char * &data, unsigned int size)
{
	size += BUFFER_ARR_SIZE_MIN;
	unsigned int newSize = BUFFER_ARR_SIZE_MIN;
	while(newSize < size)
		newSize <<= 1;
	data = new char[newSize];
	memset(data, 0, newSize);

	return newSize;
}

void BuffParserPack::onRead()
{
	evbuffer * pBuffer = bufferevent_get_input(_pBufferEvent);

	while (true)
	{
		size_t buffer_len = evbuffer_get_length(pBuffer);

		if ( buffer_len < BUFFER_HEAD_LEN)
			break;

		char pHead[BUFFER_HEAD_LEN];

		evbuffer_copyout(pBuffer, pHead, BUFFER_HEAD_LEN);

		unsigned int msglen = *(unsigned int*)(pHead);
		//msglen = int(pHead);
		parserHead(pHead, &msglen);

		if (msglen == 0)
		{
			onReadFailFun();
			break;
		}

		if ( buffer_len < msglen + BUFFER_HEAD_LEN)
			break;

		evbuffer_drain(pBuffer, BUFFER_HEAD_LEN);
		//_data = new char[msglen + 1];
		newBuff(_data, msglen);
		_data[msglen] = 0;
		_total = msglen;
		evbuffer_remove(pBuffer, _data, msglen);
		this->onReadCallBack();
	}

	return;
}

bool BuffParserPack::sendData(char * data, int len)
{
	if ( !_pBufferEvent ) return false;

	if (-1 == bufferevent_write(_pBufferEvent, &len, BUFFER_HEAD_LEN))
		return false;

	if (-1 == bufferevent_write(_pBufferEvent, data, len))
		return false;

	return true;
}

void BuffParserPack::clear()
{
	_data = NULL;
	_total = 0;
	BuffParser::clear();
}