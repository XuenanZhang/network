/************************************************************************/
/*								网络监听器								*/
/************************************************************************/
#ifndef _LISTENER_H_
#define _LISTENER_H_

#include "CommonDefine.h"
#include "log.h"
#include "NetDefine.h"
#if defined(_LINUX)
#include <string.h>
#endif

class Listener
{
public:
	Listener();
	virtual ~Listener();

public:
	/**
	 *	初始化
	 */
	virtual bool init() = 0;

	/**
	 *	开始监听
	 */
	virtual void listen() = 0;

	/**
	 *	关闭
	 */
	virtual bool shutDown() = 0;

	/**
	 *	设置连接IP地址
	 */
	void setIP( const String &strIP ) { strncpy(_ip, strIP.c_str(), strIP.length()); }
	void setIP( const char * strIP ) { strncpy(_ip, strIP, strlen(strIP)); }
	const char * getIP() { return _ip; }

	/**
	 *	设置监听端口
	 */
	void setPort( const UInt port ) { _port = port; }
	int getPort() { return _port; }

	/**
	 *	设置接收模型
	 */
	void setReceiveModel( ReceiveModel val ) { _rModel = val; };
	ReceiveModel getReceiveModel() { return _rModel; };

	/**
	 *	断开某一个网络id连接
	 */
	virtual bool disconnectByNetId(int netId) = 0;

	/**
	 *	断开所有客户端连接
	 */
	virtual void disconnectAll() = 0;

	/**
	 *	获得本机ip
	 */
	virtual char * getLocalIp() = 0;

	/**
	 *	卸载
	 */
	virtual void dispose();

public:
	/**
	 *	从网络获取数据， 还有数据返回true否则false
	 */
	virtual bool getNetData(NetData & netData) = 0;

	/**
	 *	发送数据
	 */
	virtual void sendData(int netId, char * data, int len) = 0;

protected:
	Log * _log;
	bool _isInit;
	bool _closed;
	char _ip[32];
	UInt _port;
	ReceiveModel _rModel;
};

#endif