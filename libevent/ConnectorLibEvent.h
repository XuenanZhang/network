/************************************************************************/
/*						基于libEvent客户端封装							*/
/************************************************************************/
#ifndef _CONNECTORLIBEVENT_H_
#define _CONNECTORLIBEVENT_H_

#include "NetDefine.h"
#include "Connector.h"
#include "Pool.h"
#include "RingQueue.h"
#include <event2/util.h>

class NetMessage;
class BuffParser;
struct bufferevent;
struct event_base;
struct event;
class ConnectorLibEvent: public Connector
{
public:
	ConnectorLibEvent();
	virtual ~ConnectorLibEvent();

public:
	virtual bool init();
	virtual bool connect();
	virtual void disconnect();
	virtual bool reconnect();
	virtual void dispose();

public:
	virtual bool getNetData(NetData & netData);
	virtual void sendData(char * data, int len);

private:
	bool startup();
	bool initBaseData();
	bool loadSocketLib();
	bool loadLibEvent();
	bool setupConnector();
	void doReconnect();
	void processQueue();
	void onTick();
	void onConnect(bool bRes);
	void onReadComplete();
	void onReadFail();
	

	const char * getErrorToStr() {return evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());};


private:
	event_base * _evBase;
	event * _loopEvent;
	bufferevent *_buffevent;
	BuffParser * _buffParser;
	RingQueue<NetMessage> _netMessageQueue;
	Pool<NetMessage> _netMessagePool;	
	RingQueue<NetMessage> _opQueue;
	NetMessage * _netMsgCache;

private:
	friend static void loop_event_cb(evutil_socket_t, short, void *ctx);
	friend static void connect_cb(struct bufferevent *bev, short events, void *ctx);	
	friend static void buffParser_read_compete( BuffParser * pBuff, void *ptr);
	friend static void buffParser_Fail( BuffParser * pBuff, void *ptr);
};


#endif