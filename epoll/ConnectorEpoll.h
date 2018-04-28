/************************************************************************/
/*								Linux客户端								*/
/************************************************************************/
#ifndef _CONNECTOREPOLL_H_
#define _CONNECTOREPOLL_H_

#if defined(_LINUX)
#include <deque>
#include "log.h"
#include "PoolObj.h"
#include "Pool.h"
#include "NetDefine.h"
#include "BuffParserUtil.h"
#include "Connector.h"
#include "RingQueue.h"
#include "NetMessage.h"
#include "RingBuffer.h"
#include "SocketAPI.h"

using namespace std;

class ConnectorEpoll : public Connector
{
public:
	ConnectorEpoll();
	~ConnectorEpoll();

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
	/** 
	 * 启动网络相关信息
	 */
	bool LoadEpollLib();
	bool initSocketConnect();
	bool setupIOWorkers();
	bool initBaseData();

	/** 
	 * 处理接受数据
	 */
	bool processRecvData();

	/** 
	 * 处理发送数据
	 */
	bool processSendData();

	void onReconnect();

	static void * recvThread( void * param );
	static void * sendThread( void * param );

	string getErrorInfo();

private:
	bool _isClosed;
	SOCKET _socket;
	RingBuffer _recvBuff;
	RingBuffer _sendBuff;
	BuffParserUtil _buffIOCP;
	char _cacheRecvBuff[MAX_BUFFER_LEN];
	char _cacheSendBuff[MAX_BUFFER_LEN];

	RingQueue<NetMessage> _netMessageQueue;
	Pool<NetMessage> _netMessagePool;	
	NetMessage * _netMsgCache;
};

#endif
#endif