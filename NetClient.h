/************************************************************************/
/*								  客户端连接器							*/
/************************************************************************/
#ifndef _NETSERVER_H_
#define _NETSERVER_H_

#include "CommonDefine.h"
#include "NetDefine.h"
#include "log.h"
#include "Thread.h"

class Connector;
class NetClient : public Thread
{
public:
	NetClient();
	virtual ~NetClient();

public:
	/**
	 *	初始化服务器
	 */
	bool init();

	/**
	 *	断开连接
	 */
	bool disconnect();

	/**
	 *	是否连接中
	 */
	bool isConnecting();

	/**
	 *	重新连接
	 */
	bool reconnect();

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

public:
	/**
	 *	从网络获取数据， 还有数据返回true否则false
	 */
	bool getNetData(NetData & netData);

	/** 发送数据 **/
	void sendData(char * data, int len);

protected:
	void onExit();
	void run();

private:
	Connector * _connector;
	Log * _log;
	bool _isInit;
	char _ip[32];
	UInt _port;
	ReceiveModel _rModel;

private:


};

#endif