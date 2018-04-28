#if defined(_WINDOWS)
#include "ListenerIOCP.h"
#include "CommonDefine.h"
#include "NetMessage.h"
#include "Utils.h"
#include "SocketAPI.h"

//释放socket
#define  RELEASE_SOCKET(x) {if(x!=INVALID_SOCKET){closesocket(x); x= INVALID_SOCKET;}}

ListenerIOCP::ListenerIOCP()
{
	_shutDownEvent = NULL;
	_ioCompletionPort = NULL;
	_pWorkThread = NULL;
	_threadNum = 0;
	_pListenSocket = NULL;
	_lpfnAcceptEx = NULL;
	_lpfnGetAcceptExSockAddrs = NULL;
	_currNetId = 0;
	_errorStr = NULL;
	_closed = false;
}

ListenerIOCP::~ListenerIOCP()
{

}

bool ListenerIOCP::init()
{
	if (_isInit)
	{
		_log->printfLog(LogType_Error, "[net] server had init complete");
		return false;
	}

	_log->printfLog(LogType_Normal, "[net] start startup server...");

	if (!startup())
	{
		_log->printfLog(LogType_Error, "[net] server startup fail !");
		return false;
	}
	_isInit = true;

	_log->printfLog(LogType_Normal, "[net] init server success...");
	return true;
}

void ListenerIOCP::listen()
{
	while ( !_closed )
	{
		Utils::sleep(1000);
	}
}

bool ListenerIOCP::shutDown()
{
	_closed = true;
	return true;
}
void ListenerIOCP::dispose()
{
	if ( _pListenSocket && _pListenSocket->socket)
	{
		SetEvent(_shutDownEvent);

		for (int i = 0; i < _threadNum; ++i)
		{
			PostQueuedCompletionStatus(_ioCompletionPort, 0, NULL, NULL);
		}

		WaitForMultipleObjects(_threadNum, _pWorkThread, true, INFINITE);

		for (int i = 0; i < _threadNum; ++i)
		{
			SAFE_DEL_HANDLE(_pWorkThread[i]);
		}
		_threadNum = 0;

		SAFE_DEL_ARR(_pWorkThread);

		for (int i = 0; i < MAX_CONNECT_NUM; ++i)
		{
			if (_arrSocketContext[i] != NULL)
			{
				disconnectSocket(_arrSocketContext[i]);
			}
		}

		SAFE_DEL_HANDLE(_shutDownEvent);

		SAFE_DEL_HANDLE(_ioCompletionPort);

		RELEASE_SOCKET(_pListenSocket->socket);

		SAFE_DEL(_pListenSocket);

		_lpfnAcceptEx = NULL;

		_lpfnGetAcceptExSockAddrs = NULL;

		_pIOList.clear();

		_socketContextPool.clear();

		_netMessagePool.clear();

		_netMessageQueue.clear();

		unLoadSocketLib();
	}
	Listener::dispose();
}

bool ListenerIOCP::disconnectSocket(SocketContext * pSocketContext)
{
	SAFE_RET_VAL(pSocketContext, _pListenSocket, false);
	SAFE_RET_VAL(pSocketContext->socket, INVALID_SOCKET, false);
	_log->printfLog(LogType_Normal, "client disconnect netId = %d ! ip = %s", pSocketContext->netId, pSocketContext->ip);
	//notifyDisConnection(pSocketContext->netId, pSocketContext->ip, pSocketContext->port);

	if (pSocketContext->isNetmessage)
	{
		NetMessage * pMsg = _netMessagePool.newObj();
		pMsg->netId = pSocketContext->netId;
		pMsg->state = NetState_Close;
		_netMessageQueue.push(pMsg);
	}

	freeSockeContext(pSocketContext);
	return true;
}

bool ListenerIOCP::disconnectByNetId(int netId)
{
	SocketContext * pSocketContext = getSocketContext(netId);
	if (!pSocketContext)
	{
		_log->printfLog(LogType_Error, "disconnectSocket fail netId = %d not exist", netId);
		return false;
	}

	sendData(netId, 0, 0);
	//_log->printfLog(LogType_Normal, "client disconnect netId = %d !", netId);
	//freeSockeContext(pSocketContext);
	return true;
}

void ListenerIOCP::disconnectAll()
{
	for (int i = 0; i < MAX_CONNECT_NUM; ++i)
	{
		if (_arrSocketContext[i] != NULL)
		{
			disconnectByNetId(_arrSocketContext[i]->socket);
		}
	}
}

char * ListenerIOCP::getLocalIp()
{
	string str = "";
	hostent* host;
	char* ip;
	host = gethostbyname("");
	ip = inet_ntoa (*(struct in_addr *)*host->h_addr_list);
	return ip; 
}

bool ListenerIOCP::startup()
{
	if ( !LoadSocketLib() ) return false;

	if ( !createCompletionPort() ) return false;

	if ( !setupListenSocket() ) return false;

	if ( !setupFunctionPtr() ) return false;

	if ( !initBaseData() ) return false;

	if ( !setupIOWorkers() ) return false;

	return true;
}

bool ListenerIOCP::LoadSocketLib()
{
	_log->printfLog(LogType_Normal, "Load Socket Lib...");

	WSADATA wsaData;
	int ret;
	ret = WSAStartup(MAKEWORD(2,2), &wsaData); //请求winsock库
	if (NO_ERROR != ret)
	{
		_log->printfLog(LogType_Error, "[net] request SocketLib fail in LoadSocketLib : %s", getErrorInfo());
		return false; 
	}

	return true;
}

bool ListenerIOCP::createCompletionPort()
{
	_log->printfLog(LogType_Normal, "[net] create completion port...");

	_ioCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (NULL == _ioCompletionPort)
	{
		_log->printfLog(LogType_Error, "[net] init CompletionPort fail in createCompletionPort : %s", getErrorInfo());
		return false; 
	}

	return true;
}

bool ListenerIOCP::setupListenSocket()
{
	_log->printfLog(LogType_Normal, "[net] setup listen Socket... ip = %s port = %d", _ip, _port); 

	//创建监听socket
	_pListenSocket = new SocketContext();
	_pListenSocket->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	SocketAPI::setNonblocking(_pListenSocket->socket, true);
	SocketAPI::setReuseAddr(_pListenSocket->socket, true);

	if (_pListenSocket->socket == INVALID_SOCKET)
	{
		_log->printfLog(LogType_Error, "[net] init Listen Socket fail in setupListenSocket : %s", getErrorInfo());
		return false; 
	}

	// 将Listen Socket绑定至完成端口中
	if ( NULL == CreateIoCompletionPort((HANDLE)_pListenSocket->socket, _ioCompletionPort, (DWORD)_pListenSocket, 0) )
	{
		_log->printfLog(LogType_Error, "[net] init bind completionPort fail in setupListenSocket : %s", getErrorInfo());
		return false;
	}

	sockaddr_in serverAddress;
	ZeroMemory(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(_ip);
	serverAddress.sin_port = htons(_port);

	// 绑定地址和端口
	if ( SOCKET_ERROR == bind(_pListenSocket->socket, (sockaddr*)&serverAddress, sizeof(serverAddress)))
	{
		_log->printfLog(LogType_Error, "[net] bind Socket fail in setupListenSocket : %s", getErrorInfo());
		return false;
	}

	if (SOCKET_ERROR == ::listen(_pListenSocket->socket, MAX_LISTEN_NUM))
	{
		_log->printfLog(LogType_Error, "[net] bind Socket fail in setupListenSocket : %s", getErrorInfo());
		return false;
	}

	return true;
}

bool ListenerIOCP::setupFunctionPtr()
{
	_log->printfLog(LogType_Normal, "[net] setup function pointer...");

	//使用AccpetEx函数前要先获取指针，因为是winSocket2范围之外的扩展函数，直接调用会慢
	// AcceptEx 和 GetAcceptExSockaddrs 的GUID，用于导出函数指针
	GUID guidAcceptEx = WSAID_ACCEPTEX;  
	GUID guidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 
	DWORD dwBytes = 0;
	if ( SOCKET_ERROR == WSAIoctl( _pListenSocket->socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, 
								   sizeof(guidAcceptEx), &_lpfnAcceptEx, sizeof(_lpfnAcceptEx), &dwBytes, NULL, NULL) )
	{
		_log->printfLog(LogType_Error, "[net] get AcceptEx function pointer fail in setupFunctionPtr : %s", getErrorInfo());
		return false;
	}


	if ( SOCKET_ERROR == WSAIoctl( _pListenSocket->socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidGetAcceptExSockAddrs, 
								   sizeof(guidGetAcceptExSockAddrs), &_lpfnGetAcceptExSockAddrs, sizeof(_lpfnGetAcceptExSockAddrs), &dwBytes, NULL, NULL) )
	{
		_log->printfLog(LogType_Error, "[net] get AcceptExSockAddrs function pointer fail in setupFunctionPtr : %s", getErrorInfo());
		return false;
	}

	return true;
}

bool ListenerIOCP::initBaseData()
{
	_socketContextPool.init(1000,500,false,MAX_CONNECT_NUM);

	_netMessageQueue.init(DEFAULT_NETMESSAGE_MAX_SERVER, 0, true);
	_netMessagePool.init(DEFAULT_NETMESSAGE_MAX_SERVER, 0, true);

	for (int i = 0; i < MAX_CONNECT_NUM; ++i)
		_arrSocketContext[i] = NULL;

	// 建立系统退出的事件通知
	_shutDownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	return true;
}

bool ListenerIOCP::setupIOWorkers()
{
	_log->printfLog(LogType_Normal, "[net] setup IOWorkers...");

	//投递多个AcceptEx请求用于接收连接
	for (int i = 0; i < MAX_POST_ACCEPT; ++i)
	{
		IOContext *pIOContext = new IOContext();
		pIOContext->type = IOAccept;
		if ( postAccept(pIOContext) == false )
		{
			_log->printfLog(LogType_Error, "[net] init IOWorkers fail in setupIOWorkers : %s", getErrorInfo());
			return false;
		}
		_pIOList.push_back(pIOContext);
	}

	//初始化工作线程
	_threadNum = getProcessorsNum() * 2;

	DWORD threadID;
	_pWorkThread = new HANDLE[_threadNum];
	for (int i = 0; i < _threadNum; ++i)
	{
		ThreadParam  *tp = new ThreadParam();
		tp->pNetServer = this;
		tp->threadId = i;
		_pWorkThread[i] = ::CreateThread(0, 0, workerThread, (void*)tp, 0, &threadID);
	}

	_log->printfLog(LogType_Normal, "[net] create threadNum = %d", _threadNum);

	return true;
}

DWORD WINAPI ListenerIOCP::workerThread(LPVOID lpParam)
{
	ThreadParam * tp = (ThreadParam *) lpParam;
	ListenerIOCP * pNetServer = tp->pNetServer;
	int threadId = tp->threadId;
	Log *_log = tp->pNetServer->_log;
	_log->printfLog(LogType_Normal, "[net] thread start work id = %d", threadId);

	OVERLAPPED *pOverLapped = NULL;
	SocketContext * pSocketContext = NULL;
	IOContext * pIOContext = NULL;
	DWORD dwBytes = 0;

	while (WAIT_OBJECT_0 != WaitForSingleObject(pNetServer->_shutDownEvent,0))
	{
		BOOL ioRet = GetQueuedCompletionStatus(pNetServer->_ioCompletionPort, &dwBytes, (PULONG_PTR)&pSocketContext, &pOverLapped, INFINITE);

		if (dwBytes == 0 && pSocketContext == NULL && pOverLapped == NULL)
			break;

		if (!ioRet)
		{
			DWORD dwIOError = GetLastError();

			if (dwIOError != WAIT_TIMEOUT)
			{
				//_log->printfLog(LogType_Error, "[net] GetQueuedCompletionStatus fail in workerThread id = %d : %s", threadId, pNetServer->getErrorInfo());
				if (dwIOError ==ERROR_NETNAME_DELETED)
				{
				}

				if (pSocketContext)
				{
					pNetServer->disconnectSocket(pSocketContext);
					continue;
				}
			}
			else
				_log->printfLog(LogType_Error, "[net] net wait timeout on GetQueuedCompletionStatus in workerThread threadId = %d", threadId);

			if ( pOverLapped)
			{
				pIOContext = CONTAINING_RECORD(pOverLapped, IOContext, overLapped);
				if (pIOContext && pIOContext->type != IOInvalid )
				{
					if ( pIOContext->type == IOAccept )
						pNetServer->postAccept(pIOContext);
					else if ( pIOContext->type == IORead)
						pNetServer->postRead(pSocketContext);
					else if ( pIOContext->type == IOWrite)
						pNetServer->postWrite(pSocketContext);
					else if ( pIOContext->type == IOWriteCompleted)
						pNetServer->postWriteCompleted(pSocketContext);
				}
			}

			continue;
		}

		if (!pOverLapped)
		{
			_log->printfLog(LogType_Error, "[net] pOverLapped == NULL in workerThread threadId = %d", threadId);
			continue;
		}

		pIOContext = CONTAINING_RECORD(pOverLapped, IOContext, overLapped);

		if (dwBytes == 0 && pIOContext->type != IOAccept)
		{
			if (pIOContext->type == IOWrite)
			{
				pSocketContext->isNetmessage = false;
				pNetServer->disconnectSocket(pSocketContext);
			}
			else if (pIOContext->type == IORead || pIOContext->type == IOWriteCompleted)
			{
				pNetServer->disconnectSocket(pSocketContext);
			}
			else
				_log->printfLog(LogType_Error, "[net] dwBytes == 0 in workerThread type = %d threadId = %d", pIOContext->type,threadId);
			continue;
		}

		//if (pIOContext->type == IORead || pIOContext->type == IOWrite)
		pIOContext->len = dwBytes;	

		pNetServer->processIOMessage(pSocketContext, pIOContext);
	}

	_log->printfLog(LogType_Normal, "[net] exit workerThread threadId = %d", threadId);
	SAFE_DEL(lpParam);

	return 0;
}

bool ListenerIOCP::processIOMessage(SocketContext * pSocketContext, IOContext * pIOContext)
{
	SAFE_RET_VAL( pSocketContext, NULL, false);
	SAFE_RET_VAL( pIOContext, NULL, false);

	bool ret = true;
	switch (pIOContext->type)
	{
	case IOAccept:
		ret = onAccept(pSocketContext, pIOContext);
		break;
	case IORead:
		ret = onRead(pSocketContext);
		break;
	case IOWrite:
		ret = onWrite(pSocketContext);
		break;
	case IOWriteCompleted:
		ret = onWriteCompleted(pSocketContext);
		break;
	default:
		_log->printfLog(LogType_Error, "[net] pIOContext->type = %d in workerThread ");
		::Sleep(30);
		ret = false;
		break;
	}

	return ret;
}

bool ListenerIOCP::onAccept( SocketContext * pSocketContext, IOContext * pIOContext)
{
	sockaddr_in * clientAddr = NULL;
	sockaddr_in * localAddr = NULL;
	int clientLen = sizeof(sockaddr_in);
	int localLen = sizeof(sockaddr_in);

	//_lpfnGetAcceptExSockAddrs( pIOContext->buff, pIOContext->wsabuf.len - ((sizeof(sockaddr_in) + 16)*2), sizeof(sockaddr_in)+16,
							   //sizeof(sockaddr_in) + 16, (LPSOCKADDR*)&localAddr, &localLen, (LPSOCKADDR*)&clientAddr, &clientLen);
	_lpfnGetAcceptExSockAddrs( pIOContext->buff, 0, sizeof(sockaddr_in)+16,
		sizeof(sockaddr_in) + 16, (LPSOCKADDR*)&localAddr, &localLen, (LPSOCKADDR*)&clientAddr, &clientLen);
	char * clientIp = inet_ntoa(clientAddr->sin_addr);
	//unsigned int clientPort = ntohl(clientAddr->sin_port);
	unsigned int clientPort = clientAddr->sin_port;
	SOCKET socket = pIOContext->socket;
	postAccept(pIOContext);

	SocketContext *pNewSc = newSocketContext();

	_log->printfLog(LogType_Normal, "[net] client connect success nedId = %d, ip = %s:%d, info =  %s ", pNewSc->netId, clientIp, clientPort, pIOContext->buff);

	if ( pNewSc == NULL )
	{
		_log->printfLog(LogType_Warning, "[net] socket connect num max ! ip = %s:%d ", clientIp, clientPort);
		//disconnectSocket(pNewSc);
		return false;
	}

	pNewSc->setSocket(socket);
	unsigned long on = 1;
	ioctlsocket(socket, FIONBIO, &on);
	memcpy(&(pNewSc->clientAddr), clientAddr, clientLen);
	memcpy(pNewSc->ip, clientIp, strlen(clientIp) );
	pNewSc->port = htons(clientPort);
	if ( NULL == CreateIoCompletionPort((HANDLE)pNewSc->socket, _ioCompletionPort, (DWORD)pNewSc,0) )
	{
		_log->printfLog(LogType_Error, "[net] bind CompletionPort fail in onAccept ");
		freeSockeContext(pNewSc);
		return false;
	}

	//notifyNewConnection(pNewSc->netId, clientIp, (int)clientPort);
	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->netId = pNewSc->netId;
	pMsg->state = NetState_Connect;
	_netMessageQueue.push(pMsg);

	/*IOContext *pNewIo = newIOContext(pNewSc);
	pNewIo->type = IORead;
	pNewIo->socket = pNewSc->socket;*/

	if ( false == postRead(pNewSc) )
		return false;

	return true;
}

bool ListenerIOCP::onRead( SocketContext * pSocketContext)
{
	IOContext * pIOContext = &pSocketContext->readIO;
	//notifyReceivedData(pSocketContext->netId, pIOContext->wsabuf.buf, pIOContext->len);
	bool ret = pSocketContext->readIO.ringBuff.write(pIOContext->wsabuf.buf, pIOContext->len);
	if (!ret)
	{
		_log->printfLog(LogType_Warning, "[net] readIO is full on onRead ");
		disconnectSocket(pSocketContext);
		return false;
	}

	char *p = NULL;
	int len = 0;
	while(BuffParserUtil::getDataPacket(pSocketContext->readIO.ringBuff, p, len))
	{
		NetMessage * pMsg = _netMessagePool.newObj();
		if (!pMsg)
		{
			_log->printfLog(LogType_Warning, "[net] netMessagePool is full on onRead ");
			disconnectSocket(pSocketContext);
			return false;
		}
		pMsg->netId = pSocketContext->netId;
		pMsg->state = NetState_Receive;
		pMsg->data = p;
		pMsg->len = len;
		_netMessageQueue.push(pMsg);
	}
	return postRead(pSocketContext);
}

bool ListenerIOCP::onWrite( SocketContext * pSocketContext)
{
	IOContext * pIOContext = &pSocketContext->writeIO;
	pIOContext->type = IOWriteCompleted;
	int size = pIOContext->ringBuff.getUsedSize();
	if ( size == 0 && pSocketContext->isClosed )
	{
		pSocketContext->isNetmessage = false;
		disconnectSocket(pSocketContext);
		return true;
	}

	return postWriteCompleted(pSocketContext);
}

bool ListenerIOCP::onWriteCompleted( SocketContext * pSocketContext)
{
	IOContext * pIOContext = &pSocketContext->writeIO;
	pIOContext->ringBuff.remove(pIOContext->len);
	//_log->printfLog(LogType_Normal, "[net] netId = %d remove buff size = %d, free = %d", pSocketContext->netId,pIOContext->len, pIOContext->ringBuff.getFreeSize());
	int size = pIOContext->ringBuff.getUsedSize();
	if (size > 0)
		postWriteCompleted(pSocketContext);
	else
	{
		if (pSocketContext->isClosed)
		{
			pSocketContext->isNetmessage = false;
			disconnectSocket(pSocketContext);
		}
		else
			pSocketContext->isSend = false;
	}
	return true;
}


bool ListenerIOCP::postAccept(IOContext * pIOContext)
{
	pIOContext->resetBuff();

	if ( _pListenSocket->socket == INVALID_SOCKET )
	{
		_log->printfLog(LogType_Error, "[net] _pListenSocket->socket = INVALID_SOCKET in postAccept ");
		return false;
	}

	if ( pIOContext->type != IOAccept )
	{
		_log->printfLog(LogType_Warning, "[net] pIOContext->type != IOAccept in postAccept ");
		pIOContext->type = IOAccept;
	}

	DWORD dwWord = 0;
	pIOContext->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (pIOContext->socket == INVALID_SOCKET)
	{
		_log->printfLog(LogType_Error, "[net] WSASocket fail! in postAccept :%s",getErrorInfo());
		return false;
	}

	SocketAPI::setNonblocking(pIOContext->socket, true);
	SocketAPI::setLinger(pIOContext->socket, true);
	SocketAPI::setSendBuffSize(pIOContext->socket, MAX_BUFFER_LEN);
	SocketAPI::setRecvBuffSize(pIOContext->socket, MAX_BUFFER_LEN);

	if ( false == _lpfnAcceptEx(_pListenSocket->socket, pIOContext->socket, &pIOContext->buff,0, 
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwWord, &pIOContext->overLapped) )
	//if ( false == _lpfnAcceptEx(_pListenSocket->socket, pIOContext->socket, &pIOContext->buff,pIOContext->wsabuf.len - ((sizeof(sockaddr_in) + 16)*2), 
								//sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwWord, &pIOContext->overLapped) )
	{
		if(ERROR_IO_PENDING != WSAGetLastError())
		{
			_log->printfLog(LogType_Error, "[net] run function AcceptEx fail in postAccept :%s",getErrorInfo());
			return false;
		}
	}
	
	return true;
}

bool ListenerIOCP::postRead(SocketContext * pSocketContext)
{
	SAFE_RET_VAL(pSocketContext->socket, INVALID_SOCKET, false);
	IOContext * pIOContext = &pSocketContext->readIO;
	if ( pIOContext->type != IORead )
	{
		_log->printfLog(LogType_Error, "[net] pIOContext->type != IORead in postRead ");
		return false;
	}

	pIOContext->resetBuff();

	int ret = WSARecv(pIOContext->socket, &pIOContext->wsabuf, 1, &pIOContext->dwBytes, &pIOContext->dwFlags, &pIOContext->overLapped, NULL);

	if (ret == SOCKET_ERROR && ERROR_IO_PENDING != WSAGetLastError())
	{
		_log->printfLog(LogType_Error, "[net] WSARecv fail in postRead netId = %d :%s",pSocketContext->netId,getErrorInfo());
		disconnectSocket(pSocketContext);
		return false;
	}

	return true;
}

bool ListenerIOCP::postWrite(SocketContext * pSocketContext)
{
	SAFE_RET_VAL(pSocketContext->socket, INVALID_SOCKET, false);
	IOContext * pIOContext = &pSocketContext->writeIO;
	if ( pIOContext->type != IOWrite )
	{
		_log->printfLog(LogType_Warning, "[net] pIOContext->type != IOWrite in postWrite ");
		pIOContext->type = IOWrite;
	}

	DWORD dwBytes = pIOContext->ringBuff.getUsedSize();//pIOContext->wsabuf.len;
	BOOL result = PostQueuedCompletionStatus(_ioCompletionPort, dwBytes, (DWORD)pSocketContext, &pIOContext->overLapped);
	if ( !result )//&& ERROR_IO_PENDING != GetLastError())
	{
		_log->printfLog(LogType_Error, "[net] PostQueuedCompletionStatus fail in postWrite :%s",getErrorInfo());
		disconnectSocket(pSocketContext);
		return false;
	}
	
	return true;
}

bool ListenerIOCP::postWriteCompleted(SocketContext * pSocketContext)
{
	SAFE_RET_VAL(pSocketContext->socket, INVALID_SOCKET, false);
	IOContext * pIOContext = &pSocketContext->writeIO;
	pIOContext->type = IOWriteCompleted;

	int size = pIOContext->ringBuff.getUsedSize();

	pIOContext->ringBuff.copy(pIOContext->buff, MAX_BUFFER_LEN, size);
	pIOContext->wsabuf.len = size;
	pIOContext->dwBytes = size;

	int ret = WSASend(pSocketContext->socket, &pIOContext->wsabuf, 1, &pIOContext->dwBytes, pIOContext->dwFlags, &pIOContext->overLapped, NULL);

	if ( ret == SOCKET_ERROR && WSA_IO_PENDING != WSAGetLastError() )
	{
		_log->printfLog(LogType_Error, "[net] WSASend fail in postWriteCompleted netId = %d :%s",pSocketContext->netId,getErrorInfo());
		disconnectSocket(pSocketContext);
		return false;
	}

	return true;
}

SocketContext* ListenerIOCP::newSocketContext()
{
	CREATE_MUTEX(&_socketContextLock);
	SocketContext* ptr = _socketContextPool.newObj();
	
	if ( ptr == NULL )
	{
		_log->printfLog(LogType_Error, "[net] SocketContext Pool full in newSocketContext");
		return NULL;
	}

	while (true)
	{
		if (_currNetId >= MAX_CONNECT_NUM)
			_currNetId = 0;

		if ( _arrSocketContext[_currNetId] == NULL )
		{
			_arrSocketContext[_currNetId] = ptr;
			ptr->netId = _currNetId;
			++_currNetId;
			break;
		}
		else
		{
			++_currNetId;
		}
	}

	return ptr;
}

void ListenerIOCP::freeSockeContext(SocketContext * pSocketContext)
{
	SAFE_RET( pSocketContext, NULL);
	CREATE_MUTEX(&_socketContextLock);

	if (pSocketContext->netId >= 0 && pSocketContext->netId < MAX_CONNECT_NUM)
		_arrSocketContext[pSocketContext->netId] = NULL;

	_socketContextPool.releaseObj(pSocketContext);
} 


SocketContext * ListenerIOCP::getSocketContext(int netId)
{
	if (netId < 0 || netId >= MAX_CONNECT_NUM)
		return NULL;

	return _arrSocketContext[netId];
}

bool ListenerIOCP::getNetData(NetData & netData)
{
	SAFE_RET_VAL(_closed, true, false);
	SAFE_RET_VAL(_isInit, false, false);
	if (_netMsgCache)
	{
		_netMessagePool.releaseObj(_netMsgCache);
		_netMsgCache = NULL;
	}

	NetMessage * pMsg = _netMessageQueue.pop();
	if (!pMsg) return false;
	netData.netId = pMsg->netId;
	netData.state = pMsg->state;
	netData.data = pMsg->data;
	netData.len = pMsg->len;
	_netMsgCache = pMsg;
	return true;
}

void ListenerIOCP::sendData(int netId, char * data, int len)
{
	SAFE_RET(_closed, true);
	SAFE_RET(_isInit, false);
	SocketContext * pSocketContext = getSocketContext(netId);

	if ( !pSocketContext )
	{
		_log->printfLog(LogType_Warning, "[net] netId = %d not exist", netId);
		return;
	}

	if ( MAX_PACKET_SIZE < len + BUFFER_HEAD_LEN )
	{
		_log->printfLog(LogType_Warning, "[net] netId = %d send data len > MAX_PACKET_SIZE", netId);
		return;
	}


	IOContext * pIOContext = &pSocketContext->writeIO;
	pIOContext->type = IOWrite;
	
	if (len == 0)
	{
		pSocketContext->isClosed = true;
	}
	else if (!BuffParserUtil::writeDataPacket( pIOContext->ringBuff, data, len))
	{
		_log->printfLog(LogType_Warning, "[net] writeIO is full on sendData ");
		pSocketContext->isClosed = true;
	}

	if (!pSocketContext->isSend)
	{
		pSocketContext->isSend = true;
		postWrite(pSocketContext);
	}
}

int ListenerIOCP::getProcessorsNum()
{
	SYSTEM_INFO si;

	GetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}

char * ListenerIOCP::getErrorInfo()
{
	DWORD dw = WSAGetLastError();
	_errorStr="";
	switch(dw)
	{
	case WSAEFAULT://optval不是进程地址空间的一个有效部分
		_errorStr="WSAEFAULT	The buf parameter is not completely contained in a valid part of the user address space.";
		break; 
	case WSAENOTCONN:
		_errorStr="WSAENOTCONN	The socket is not connected."; 
		break;
	case WSAEINTR:
		_errorStr="WSAEINTR	The (blocking) call was canceled through WSACancelBlockingCall.	"; 
		break;
	case WSAENOTSOCK:
		_errorStr=" WSAENOTSOCK	The descriptor s is not a socket."; 
		break; 
	case WSANOTINITIALISED:
		_errorStr="WSANOTINITIALISED: A successful WSAStartup call must occur before using this function.";
		break; 
	case WSAENETDOWN:
		_errorStr="WSAENETDOWN	The network subsystem has failed."; 
		break;
	case WSAEINPROGRESS:
		_errorStr="WSAEINPROGRESS	A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function."; 
		break;
	case WSAENETRESET:
		_errorStr=" WSAENETRESET	The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress."; 
		break; 
	case WSAEOPNOTSUPP:
		_errorStr="WSAEOPNOTSUPP	MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.	"; 
		break; 
	case WSAESHUTDOWN:
		_errorStr="WSAESHUTDOWN	The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH."; 
		break;
	case WSAEWOULDBLOCK:
		_errorStr=" WSAEWOULDBLOCK	The socket is marked as nonblocking and the receive operation would block.	"; 
		break; 
	case WSAEMSGSIZE:
		_errorStr=" WSAENOTSOCK		The message was too large to fit into the specified buffer and was truncated."; 
		break;
	case WSAEINVAL:
		_errorStr="WSAEINVAL	The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative.	"; 
	case WSAECONNABORTED:
		_errorStr=" 	WSAECONNABORTED	The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable."; 
		break; 
	case WSAETIMEDOUT:
		_errorStr="WSAETIMEDOUT	The connection has been dropped because of a network failure or because the peer system failed to respond.	"; 
		break; 
	case WSAECONNRESET:
		_errorStr="WSAECONNRESET Connection dropped..";
		break;

	default:
		_errorStr="unknown error";  
		break;
	}

	return _errorStr;
}
#endif