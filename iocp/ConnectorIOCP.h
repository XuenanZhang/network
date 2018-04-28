/************************************************************************/
/*							WinSock客户端								*/
/************************************************************************/
#ifndef _CONNECTORIOCP_H_
#define _CONNECTORIOCP_H_

#if defined(_WINDOWS)
#include <WinSock2.h>
#include "log.h"
#include "PoolObj.h"
#include "Pool.h"
#include "NetDefine.h"
#include "BuffParserUtil.h"
#include "Connector.h"
#include "RingQueue.h"
#include "NetMessage.h"
#include "RingBuffer.h"

using namespace std;

class ConnectorIOCP : public Connector
{
public:
	ConnectorIOCP();
	~ConnectorIOCP();

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
	bool LoadSocketLib();
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

	static DWORD WINAPI recvThread(LPVOID lpParam);
	static DWORD WINAPI sendThread(LPVOID lpParam);

	string getErrorInfo();

private:
	bool _isClosed;
	SOCKET _socket;
	RingBuffer _recvBuff;
	RingBuffer _sendBuff;
	BuffParserUtil _buffIOCP;
	char _cacheRecvBuff[MAX_BUFFER_LEN];
	char _cacheSendBuff[MAX_BUFFER_LEN];

	HANDLE _shutDownEvent;			//关闭事件句柄
	HANDLE _recvThread;
	HANDLE _sendThread;

	RingQueue<NetMessage> _netMessageQueue;
	Pool<NetMessage> _netMessagePool;	
	NetMessage * _netMsgCache;
};

#endif
#endif