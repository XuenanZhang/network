#if defined(_LINUX)
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include "ConnectorEpoll.h"
#include "Utils.h"

ConnectorEpoll::ConnectorEpoll()
{
	_socket = INVALID_SOCKET;
}

ConnectorEpoll::~ConnectorEpoll()
{
	dispose();
}

bool ConnectorEpoll::init()
{
	if ( !initBaseData() ) return false;

	_isInit = true;
	return true;
}

bool ConnectorEpoll::initBaseData()
{
	_netMessageQueue.init(DEFAULT_NETMESSAGE_MAX_CLIENT, 0, true);
	_netMessagePool.init(DEFAULT_NETMESSAGE_MAX_CLIENT, DEFAULT_NETMESSAGE_MAX_CLIENT, true);
	_netMsgCache = NULL;
	_recvBuff.initSize(MAX_BUFFER_LEN);
	_sendBuff.initSize(MAX_BUFFER_LEN);

	return true;
}

bool ConnectorEpoll::connect()
{
	initSocketConnect();
	setupIOWorkers();
	while ( _isConncet )
	{
		Utils::sleep(100);
	}

	return true;
}

bool ConnectorEpoll::reconnect()
{
	if (_reconnect) return false;
	_reconnect = true;
	disconnect();
	return true;
}

void ConnectorEpoll::onReconnect()
{
	if (_socket != INVALID_SOCKET )
	{
		SocketAPI::closeSocket(_socket);
		_socket = INVALID_SOCKET;
	}

	NetMessage * pMsg;
	while ( pMsg = _netMessageQueue.pop())
		_netMessagePool.releaseObj(pMsg);
	_recvBuff.clear();
	_sendBuff.clear();
	initSocketConnect();
	_reconnect = false;
}

void ConnectorEpoll::disconnect()
{
	if (_isClosed) return;
	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->state = NetState_Close;
	_netMessageQueue.push(pMsg);
	_isClosed = true;
}

void ConnectorEpoll::dispose()
{
	SAFE_RET(_isInit, false);

	if (_socket != INVALID_SOCKET )
	{
		SocketAPI::closeSocket(_socket);
		_socket = INVALID_SOCKET;
	}

	_recvBuff.clear();
	_sendBuff.clear();

	_isInit = false;
	_isConncet = false;
}

bool ConnectorEpoll::initSocketConnect()
{
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//SocketAPI::setSocketNonblocking(_socket);
	if (_socket == INVALID_SOCKET )
	{
		_log->printfLog(LogType_Error, "create socket fail in initSocketConnect : errno = %d", errno);
		return false; 
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(_port);
	serverAddr.sin_addr.s_addr = inet_addr(_ip);

	_log->printfLog(LogType_Normal, "start connect server... ip = %s port = %d", _ip, _port);


	//WSAEINTR  WSAEALREADY 
	if ( SOCKET_ERROR == ::connect(_socket, reinterpret_cast<const sockaddr *>(&serverAddr), sizeof(serverAddr)))
	{
		SocketAPI::closeSocket(_socket);
		_log->printfLog(LogType_Error, "connect fail in initSocketConnect : errno = %d", errno);
		return false;
	}

	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->state = NetState_Connect;
	_netMessageQueue.push(pMsg);

	_log->printfLog(LogType_Normal, "server connect success!!!");
	_isConncet = true;
	_isClosed = false;
	return true;
}

bool ConnectorEpoll::setupIOWorkers()
{
	pthread_t threadID = 0;

	pthread_create( &threadID, NULL , recvThread , this );
	_log->printfLog(LogType_Normal, "[net] create thread recv!!!");

	pthread_create( &threadID, NULL , sendThread , this );
	_log->printfLog(LogType_Normal, "[net] create thread send!!!");

	return true;
}

bool ConnectorEpoll::processRecvData()
{
	SAFE_RET_VAL(_isConncet, false, false);
	SAFE_RET_VAL(_isClosed, true, false);
	int ret = recv(_socket, _cacheRecvBuff, MAX_BUFFER_LEN, 0);
	_log->printfLog(LogType_Normal, "[net] processRecvData ret = %d", ret);
	if ( ret == SOCKET_ERROR || ret == 0 ) //为0代表网络中断
	{
		if ( !_reconnect )
		{
			disconnect();
			if (ret != 0) _log->printfLog(LogType_Error, "recv fail in processRecvData : errno = %d", errno);
		}
		return false;
	}

	if ( !_recvBuff.write(_cacheRecvBuff, ret) )
	{
		if ( !_reconnect )
		{
			disconnect();
			_log->printfLog(LogType_Error, "recv is full in processRecvData");
		}
		return false;
	}

	char *p = NULL;
	int len = 0;
	while( BuffParserUtil::getDataPacket(_recvBuff, p, len) )
	{
		NetMessage * pMsg = _netMessagePool.newObj();
		pMsg->state = NetState_Receive;
		pMsg->data = p;
		pMsg->len = len;
		_netMessageQueue.push(pMsg);
	}

	return true;
}

bool ConnectorEpoll::processSendData()
{
	SAFE_RET_VAL(_isConncet, false, false);
	SAFE_RET_VAL(_isClosed, true, false);
	unsigned int usedSize = _sendBuff.getUsedSize();
	if (usedSize == 0 ) return false;

	_sendBuff.copy(_cacheSendBuff, MAX_BUFFER_LEN, usedSize);

	int ret = send(_socket, _cacheSendBuff, usedSize, 0);
	if ( ret == SOCKET_ERROR )
	{
		if ( !_reconnect )
		{
			if (ret != 0) _log->printfLog(LogType_Error, "send fail in processSendData : errno = %d", errno);
			disconnect();
		}
		return false;
	}

	_sendBuff.remove(ret);
	return true;
}


void * ConnectorEpoll::recvThread( void * param )
{
	ConnectorEpoll * pNetClient = (ConnectorEpoll *)param;

	while (pNetClient->isConnecting())
	{
		if ( !pNetClient->processRecvData() )
			Utils::sleep(10);
	}

	return 0;
}

void * ConnectorEpoll::sendThread( void * param )
{
	ConnectorEpoll * pNetClient = (ConnectorEpoll *)param;

	while (pNetClient->isConnecting())
	{
		if ( !pNetClient->processSendData() )
			Utils::sleep(10);
	}

	return 0;
}

void ConnectorEpoll::sendData(char * data, int len)
{
	SAFE_RET(_isClosed, true);
	if (len == 0) 
	{
		_log->printfLog(LogType_Error, "send fail len = 0");
		return;
	}

	if (MAX_BUFFER_LEN < len + BUFFER_HEAD_LEN) 
	{
		_log->printfLog(LogType_Error, "send fail len >  MAX_BUFFER_LEN ");
		return;
	}

	if ( !BuffParserUtil::writeDataPacket(_sendBuff, data, len) )
	{
		_log->printfLog(LogType_Error, "send fail _sendBuff is full on sendData");
		disconnect();
		return;
	}
}

bool ConnectorEpoll::getNetData(NetData & netData)
{
	if (!_isInit || !_isConncet) return false;
	if (_netMsgCache)
	{
		int state = _netMsgCache->state;
		_netMessagePool.releaseObj(_netMsgCache);
		_netMsgCache = NULL;
		if ( state == NetState_Close )
		{
			//if (_reconnect)
			//{
			onReconnect();
			//}
			//else
			//_isConncet = false;
			return false;
		}
	}

	if (_netMessageQueue.getMaxCount() == 0) return false;

	NetMessage * pMsg = _netMessageQueue.pop();
	if (!pMsg) return false;
	netData.state = pMsg->state;
	netData.data = pMsg->data;
	netData.len = pMsg->len;
	_netMsgCache = pMsg;
	return true;
}















#endif