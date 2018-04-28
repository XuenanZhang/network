#include <event2/bufferevent.h>
#include <event2/event.h>
#include "BuffParser.h"
#include "ListenerLibEvent.h"

static void bufferRead_cb(struct bufferevent *bev, void *ctx)
{
	BuffParser * pBuff = static_cast<BuffParser *>(ctx);

	if ( pBuff && !pBuff->isClose() )
	{
		pBuff->onRead();
	}
}

static void buffferSend_cb(struct bufferevent *bev, void *ctx)
{
	BuffParser * pBuff = static_cast<BuffParser *>(ctx);

	if ( pBuff && !pBuff->isClose() )
	{
		pBuff->onWrite();
	}
}

static void buffevent_cb(struct bufferevent *bev, short what, void *ctx)
{
	BuffParser * pBuff = static_cast<BuffParser *>(ctx);
	if ( pBuff && !pBuff->isClose() )
	{
		pBuff->onReadFailFun();
	}
}

BuffParser::BuffParser():_pBufferEvent(NULL)
{
	clear();
}
BuffParser::~BuffParser()
{
	clear();
}

void BuffParser::init(bufferevent *pBev, void * master, read_cb_func readCb, fail_cb_func failCb)
{
	_pBufferEvent = pBev;
	_master = master;
	_readCb = readCb;
	_failCb = failCb;
	bufferevent_setcb(_pBufferEvent, bufferRead_cb, buffferSend_cb, buffevent_cb, this);
	bufferevent_enable(_pBufferEvent, EV_READ|EV_WRITE);
}


void BuffParser::clear()
{
	if (_pBufferEvent)
	{
		bufferevent_free(_pBufferEvent);
		_pBufferEvent = NULL;
	}

	_netId = 0;
	_master = NULL;
	_close = false;
	_readCb = NULL;
	_failCb = NULL;
}

void BuffParser::onReadCallBack()
{
	if (_readCb)
		_readCb(this, _master);
	else
		printf("readCb fun not exist£¡£¡£¡");
}

void BuffParser::onReadFailFun()
{
	close();
	if (_failCb)
		_failCb(this, _master);
	else
		printf("failCb fun not exist£¡£¡£¡");
}

