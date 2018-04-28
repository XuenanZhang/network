/************************************************************************/
/*						基于libEvent服务器封装							*/
/************************************************************************/
#ifndef _LISTENERLIBEVENT_H_
#define _LISTENERLIBEVENT_H_

#include "Listener.h"
#include "NetDefine.h"
#include "Pool.h"
#include "RingQueue.h"
#include <event2/util.h>

struct event_base;
struct evconnlistener;
struct bufferevent;
class BuffParser;
class BuffParserPack;
class NetMessage;
struct event;
class ListenerLibEvent : public Listener
{
public:
	ListenerLibEvent();
	virtual ~ListenerLibEvent();

public:
	bool init();

	void listen();

	bool shutDown();

	bool disconnectByNetId(int netId);

	void disconnectAll();

	char * getLocalIp();

	void dispose();

public:
	bool getNetData(NetData & netData);

	void sendData(int netId, char * data, int len);

protected:

private:
	/**
	 *	启动网络相关信息
	 */
	bool startup();
	bool initBaseData();
	bool loadSocketLib();
	bool loadLibEvent();
	bool setupListener();


	void onTick();

	/**
	 *	处理libevent事件信息
	 */
	void onAccept(bufferevent *pBev, const char *pIP, UInt nIP);
	void onAcceptError();
	void onReadComplete(BuffParser * pBuff);
	void onReadFail(BuffParser * pBuff);

	/**
	 *	buffParse池
	 */
	BuffParser* newBuffParser();
	void freeBuffParser(BuffParser * pBuffParser);
	BuffParser* getBuffParser(int netId);

	void processQueue();

	/**
	 *	返回错误信息
	 */
	const char * getErrorToStr() {return evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());};
	
private:
	event_base * _evBase;
	event * _loopEvent;
	evconnlistener * _evconnlistener;

	BuffParser * _arrBuffParser[MAX_CONNECT_NUM];
	Pool<BuffParserPack> _buffPackPool;

	//网络消息队列
	RingQueue<NetMessage> _netMessageQueue; 
	Pool<NetMessage> _netMessagePool;

	RingQueue<NetMessage> _opQueue;
	NetMessage * _netMsgCache;

	Int _currNetId;
	Int _totalNetNum;

private:
	friend static void buffevent_cb(struct bufferevent *bev, short what, void *ctx);
	friend static void accept_conn_cb(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * address, int socklen, void *ctx);
	friend static void accept_error_cb(struct evconnlistener *listener, void *ctx);
	friend static void buffParser_read_compete( BuffParser * pBuff, void *ptr);
	friend static void buffParser_Fail( BuffParser * pBuff, void *ptr);
	friend static void loop_event_cb(evutil_socket_t, short, void *ctx);

};


#endif