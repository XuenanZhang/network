/************************************************************************/
/*								  服务器监听器							*/
/************************************************************************/
#ifndef _NETSERVER_H_
#define _NETSERVER_H_

#include "CommonDefine.h"
#include "NetDefine.h"
#include "log.h"
#include "Thread.h"

class Listener;
class NetServer : public Thread
{
public:
	NetServer();
	virtual ~NetServer();

public:
	/**
	 *	初始化服务器
	 */
	bool init();

	/**
	 *	关服
	 */
	bool shutDown();

	/**
	 *	设置连接IP地址
	 */
	void setIP( const String &strIP );
	void setIP( const char * strIP );
	const char * getIP();

	/**
	 *	设置监听端口
	 */
	void setPort( const UInt port ) { _port = port; }
	int getPort() { return _port; }

	/**
	 *	断开某一个网络id连接
	 */
	virtual bool disconnectByNetId(int netId);

	/**
	 *	断开所有客户端连接
	 */
	virtual void disconnectAll();

	/**
	 *	获得本机ip
	 */
	virtual char * getLocalIp();

public:
	/**
	 *	从网络获取数据， 还有数据返回true否则false
	 */
	bool getNetData(NetData & netData);

	/** 发送数据 **/
	void sendData(int netId, char * data, int len);

protected:
	void onExit();
	void run();
private:
	Listener * _listener;
	Log * _log;
	bool _isInit;
	char _ip[32];
	UInt _port;
	ReceiveModel _rModel;

private:


};

#endif