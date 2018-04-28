#if defined(_WINDOWS)
#include "ConnectorIOCP.h"
#include "Utils.h"
#include "SocketAPI.h"

ConnectorIOCP::ConnectorIOCP()
{
	_socket = INVALID_SOCKET;
	_shutDownEvent = INVALID_HANDLE_VALUE;
	_recvThread = INVALID_HANDLE_VALUE;
	_sendThread = INVALID_HANDLE_VALUE;
}

ConnectorIOCP::~ConnectorIOCP()
{
	dispose();
}

bool ConnectorIOCP::init()
{
	if ( !LoadSocketLib() ) return false;

	if ( !setupIOWorkers() ) return false;

	if ( !initBaseData() ) return false;

	_isInit = true;
	return true;
}

bool ConnectorIOCP::initBaseData()
{
	_netMessageQueue.init(DEFAULT_NETMESSAGE_MAX_CLIENT, 0, true);
	_netMessagePool.init(DEFAULT_NETMESSAGE_MAX_CLIENT, DEFAULT_NETMESSAGE_MAX_CLIENT, true);
	_netMsgCache = NULL;
	_recvBuff.initSize(MAX_BUFFER_LEN);
	_sendBuff.initSize(MAX_BUFFER_LEN);

	return true;
}

bool ConnectorIOCP::connect()
{
	initSocketConnect();
	while ( _isConncet )
	{
		Utils::sleep(100);
	}

	return true;
}

bool ConnectorIOCP::reconnect()
{
	if (_reconnect) return false;
	_reconnect = true;
	disconnect();
	return true;
}

void ConnectorIOCP::onReconnect()
{
	if (_socket != INVALID_SOCKET )
	{
		CancelIo((HANDLE)_socket);
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

void ConnectorIOCP::disconnect()
{
	if (_isClosed) return;
	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->state = NetState_Close;
	_netMessageQueue.push(pMsg);
	_isClosed = true;
}

void ConnectorIOCP::dispose()
{
	SAFE_RET(_isInit, false);
	SetEvent(_shutDownEvent);

	if( _recvThread )
		WaitForSingleObject( _recvThread , INFINITE );
	if( _sendThread )
		WaitForSingleObject( _sendThread , INFINITE );

	if (_socket != INVALID_SOCKET )
	{
		CancelIo((HANDLE)_socket);
		SocketAPI::closeSocket(_socket);
		_socket = INVALID_SOCKET;
	}

	SAFE_DEL_HANDLE(_shutDownEvent);
	SAFE_DEL_HANDLE(_recvThread);
	SAFE_DEL_HANDLE(_sendThread);

	_recvBuff.clear();
	_sendBuff.clear();

	_isInit = false;
	_isConncet = false;
}

bool ConnectorIOCP::LoadSocketLib()
{
	WSADATA wsaData;
	int ret;
	ret = WSAStartup(MAKEWORD(2,2), &wsaData); //请求winsock库
	if (NO_ERROR != ret)
	{
		_log->printfLog(LogType_Error, "request SocketLib fail in LoadSocketLib : %s", getErrorInfo());
		return false; 
	}

	_log->printfLog(LogType_Normal, "Loaded Socket Lib!");
	return true;
}

bool ConnectorIOCP::initSocketConnect()
{
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//SocketAPI::setNonblocking(_socket);
	if (_socket == INVALID_SOCKET )
	{
		_log->printfLog(LogType_Error, "create socket fail in initSocketConnect : %s", getErrorInfo());
		return false; 
	}

	sockaddr_in serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(_port);
	serverAddr.sin_addr.s_addr = inet_addr(_ip);

	if (serverAddr.sin_addr.s_addr == INADDR_NONE)   
	{   
		LPHOSTENT lphost;   
		lphost = gethostbyname(_ip);   
		if (lphost != NULL)   
			serverAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;   
		else
			return false;
	} 

	_log->printfLog(LogType_Normal, "start connect server... ip = %s port = %d", _ip, _port);


	//WSAEINTR  WSAEALREADY 
	if ( SOCKET_ERROR == ::connect(_socket, reinterpret_cast<const sockaddr *>(&serverAddr), sizeof(serverAddr)))
	{
		SocketAPI::closeSocket(_socket);
		_log->printfLog(LogType_Error, "connect fail in initSocketConnect : %s", getErrorInfo());
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

bool ConnectorIOCP::setupIOWorkers()
{
	// 建立系统退出的事件通知
	_shutDownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	DWORD threadID;

	_recvThread = ::CreateThread(0, 0, recvThread, (void *)this, 0, &threadID);
	_log->printfLog(LogType_Normal, "[net] create thread recv!!!");

	_sendThread = ::CreateThread(0, 0, sendThread, (void *)this, 0, &threadID);
	_log->printfLog(LogType_Normal, "[net] create thread send!!!");

	
	return true;
}

bool ConnectorIOCP::processRecvData()
{
	SAFE_RET_VAL(_isConncet, false, false);
	SAFE_RET_VAL(_isClosed, true, false);
	int ret = recv(_socket, _cacheRecvBuff, MAX_BUFFER_LEN, 0);
	if ( ret == SOCKET_ERROR || ret == 0 ) //为0代表网络中断
	{
		if ( !_reconnect )
		{
			disconnect();
			_log->printfLog(LogType_Error, "recv fail in processRecvData : %s", getErrorInfo());
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

	//此处需要优化 可以放到getNetData
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

bool ConnectorIOCP::processSendData()
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
			_log->printfLog(LogType_Error, "send fail in processSendData : %s", getErrorInfo());
			disconnect();
		}
		return false;
	}

	_sendBuff.remove(ret);
	return true;
}


DWORD WINAPI ConnectorIOCP::recvThread(LPVOID lpParam)
{
	ConnectorIOCP * pNetClient = (ConnectorIOCP *)lpParam;

	while (WAIT_OBJECT_0 != WaitForSingleObject(pNetClient->_shutDownEvent,0))
	{
		if (!pNetClient->processRecvData())
			Utils::sleep(10);
	}

	return 0;
}

DWORD WINAPI ConnectorIOCP::sendThread(LPVOID lpParam)
{
	ConnectorIOCP * pNetClient = (ConnectorIOCP *)lpParam;

	while (WAIT_OBJECT_0 != WaitForSingleObject(pNetClient->_shutDownEvent,0))
	{
		if (!pNetClient->processSendData())
			Utils::sleep(10);
	}

	return 0;
}

void ConnectorIOCP::sendData(char * data, int len)
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

bool ConnectorIOCP::getNetData(NetData & netData)
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

string ConnectorIOCP::getErrorInfo()
{
	DWORD dw = WSAGetLastError();
	string error="";
	switch(dw)
	{
	case WSAEFAULT:
		error="WSAEFAULT	The buf parameter is not completely contained in a valid part of the user address space.";
		break; 
	case WSAENOTCONN:
		error="WSAENOTCONN	The socket is not connected."; 
		break;
	case WSAEINTR:
		error="WSAEINTR	The (blocking) call was canceled through WSACancelBlockingCall.	"; 
		break;
	case WSAENOTSOCK:
		error=" WSAENOTSOCK	The descriptor s is not a socket."; 
		break; 
	case WSANOTINITIALISED:
		error="WSANOTINITIALISED: A successful WSAStartup call must occur before using this function.";
		break; 
	case WSAENETDOWN:
		error="WSAENETDOWN	The network subsystem has failed."; 
		break;
	case WSAEINPROGRESS:
		error="WSAEINPROGRESS	A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function."; 
		break;
	case WSAENETRESET:
		error=" WSAENETRESET	The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress."; 
		break; 
	case WSAEOPNOTSUPP:
		error="WSAEOPNOTSUPP	MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.	"; 
		break; 
	case WSAESHUTDOWN:
		error="WSAESHUTDOWN	The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH."; 
		break;
	case WSAEWOULDBLOCK:
		error=" WSAEWOULDBLOCK	The socket is marked as nonblocking and the receive operation would block.	"; 
		break; 
	case WSAEMSGSIZE:
		error=" WSAENOTSOCK		The message was too large to fit into the specified buffer and was truncated."; 
		break;
	case WSAEINVAL:
		error="WSAEINVAL	The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative.	"; 
	case WSAECONNABORTED:
		error=" 	WSAECONNABORTED	The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable."; 
		break; 
	case WSAETIMEDOUT:
		error="WSAETIMEDOUT	The connection has been dropped because of a network failure or because the peer system failed to respond.	"; 
		break; 
	case WSAECONNRESET:
		error="WSAECONNRESET Connection dropped..";
		break;

	default:
		error="unknown error";  
		break;
	}

	return error;
}

#endif