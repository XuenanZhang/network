/************************************************************************/
/*						处理buffevent数据收发							*/
/************************************************************************/
#ifndef _BUFFPARSER_H_
#define _BUFFPARSER_H_

#include "CommonDefine.h"
#include "PoolObj.h"

class BuffParser;
typedef void (*read_cb_func)( BuffParser *, void *);
typedef void (*fail_cb_func)( BuffParser *, void *);

struct bufferevent;
class BuffParser : public PoolObj
{
public:
	BuffParser();
	virtual ~BuffParser();

public:
	void initByPool(){ this->clear(); };
	void releaseByPool(){ this->clear(); };

public:
	virtual void init(bufferevent *pBev, void * master, read_cb_func readCb, fail_cb_func failCb);
	virtual bool sendData(char * data, int len) = 0;

	Int getNetId() { return _netId; };
	void setNetId( Int netId ) { _netId = netId; };
	void* getMaster() { return _master; };
	void close() {_close = true;};
	bool isClose() {return _close; };

protected:
	virtual void onRead() = 0;
	virtual void onWrite() {};
	virtual void clear();
	void onReadCallBack();
	void onReadFailFun();


protected:
	bufferevent *_pBufferEvent;
	Int _netId;
	void * _master;
	read_cb_func _readCb;
	fail_cb_func _failCb;
private:
	bool _close;

private:
	friend void bufferRead_cb(struct bufferevent *bev, void *ctx);
	friend void buffferSend_cb(struct bufferevent *bev, void *ctx);
	friend void buffevent_cb(struct bufferevent *bev, short what, void *ctx);
};

#endif