/************************************************************************/
/*                     Linux服务器 基于epoll网络模型					*/
/************************************************************************/
#ifndef _LISTENEREPOLL_H_
#define _LISTENEREPOLL_H_
#define _LINUX
#if defined(_LINUX)
//#if defined(_WINDOWS)
#include "Listener.h"
#include "NetDefine.h"
#include "RingBuffer.h"
#include "PoolObj.h"
#include "Pool.h"
#include "RingQueue.h"
#include "NetMessage.h"
#include "SocketAPI.h"


typedef		int		SOCKET;
#define     INVALID_SOCKET   -1
#define		SOCKET_ERROR	 -1

class Acceptor : public PoolObj
{
public:
	SOCKET fd;
	int netId;
	RingBuffer readBuff;
	RingBuffer writeBuff;
	char ip[20];
	int port;
	bool isNetmessage;
	bool closed;
	bool isSend;
	MyLock lock;
public:
	Acceptor()
	{
		readBuff.initSize(MAX_BUFFER_LEN);
		writeBuff.initSize(MAX_BUFFER_LEN);
		clear();
	};
	~Acceptor(){};
	void initByPool(){clear(); closed = false;};
	void releaseByPool(){clear(); closed = true;};
	
	void clear()
	{
		if (fd != INVALID_SOCKET)
			SocketAPI::closeSocket(fd);
		fd = INVALID_SOCKET;
		readBuff.clear();
		writeBuff.clear();
		memset(ip, 0, 20);
		isSend = false;
		isNetmessage = true;
	}
private:

};

struct epoll_event;
class ListenerEpoll : public Listener
{
public:
	ListenerEpoll();
	~ListenerEpoll();

public:
	virtual bool init();

	virtual void listen();

	virtual bool shutDown();

	virtual bool disconnectByNetId(int netId);

	virtual void disconnectAll();

	virtual char * getLocalIp();

	void dispose();

public:
	bool getNetData(NetData & netData);

	void sendData(int netId, char * data, int len);

private:
	/**
	 *	启动网络相关信息
	 */
	bool startup();
	bool setupEpollLib();
	bool setupListenSocket();
	bool initBaseData();
	bool setupIOWorkers();

	/**
	 *	处理IO请求线程
	 */
	void workerThread();
	bool onAccept();
	bool onRead(Acceptor * pAcceptor);
	bool onWrite(Acceptor * pAcceptor, bool isChange);
	
	Acceptor * getAcceptor(int netId);
	bool disconnectSocket(Acceptor * pAcceptor);

	Acceptor* newAcceptor();
	void freeAcceptor(Acceptor * pAcceptor);
private:
	int _epollFd;
	int _listenFd;
	epoll_event * _events;
	MyLock _acceptorLock;
	char _cacheBuff[MAX_BUFFER_LEN];

	//网络消息队列
	RingQueue<NetMessage> _netMessageQueue; 
	Pool<NetMessage> _netMessagePool;
	NetMessage * _netMsgCache;

	Acceptor * _arrAcceptor[MAX_CONNECT_NUM];
	int _currNetId;
	Pool<Acceptor> _acceptorPool;
};

#endif

#endif
