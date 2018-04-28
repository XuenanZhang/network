/************************************************************************/
/*								网络连接器								*/
/************************************************************************/
#ifndef _CONNECTOR_H_
#define _CONNECTOR_H_
#include "CommonDefine.h"
#include "NetDefine.h"


class Log;
class Connector
{
public:
	Connector();
	virtual ~Connector();

public:
	/**
	 *	初始化
	 */
	virtual bool init() = 0;

	/**
	 *	开启网络连接
	 */
	virtual bool connect() = 0;

	/**
	 *	重新连接
	 */
	virtual bool reconnect() = 0;

	/**
	 *	断开连接
	 */
	virtual void disconnect() = 0;

	/**
	 *	设置连接IP地址
	 */
	void setIP( const String &strIP ) { strncpy(_ip, strIP.c_str(), strIP.length()); }
	void setIP( const char * strIP ) { strncpy(_ip, strIP, strlen(strIP)); }
	const char * getIP() { return _ip; }

	/**
	 *	设置监听端口
	 */
	void setPort( const int &port ) { _port = port; };

	/**
	 *	设置接收模型
	 */
	void setReceiveModel( ReceiveModel val ) { _rModel = val; };
	ReceiveModel getReceiveModel() { return _rModel; };

	/**
	 *	是否连接中
	 */
	bool isConnecting() { return _isConncet; };

	/**
	 *	卸载
	 */
	virtual void dispose() = 0;

public:
	/**
	 *	从网络获取数据， 还有数据返回true否则false
	 */
	virtual bool getNetData(NetData & netData) = 0;

	/**
	 *	发送数据
	 */
	virtual void sendData(char * data, int len) = 0;
protected:
	Log * _log;
	bool _isInit;
	bool _isConncet;
	bool _reconnect;
	char _ip[32];
	UInt _port;
	ReceiveModel _rModel;

};


#endif