#define _LINUX
#if defined(_LINUX)
//#if defined(_WINDOWS)
#include <sys/epoll.h>
#include <sys/resource.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>	
#include "ListenerEpoll.h"
#include "EpollAPI.h"
#include "CommonDefine.h"
#include "Utils.h"
#include "BuffParserUtil.h"
#include "Mutex.h"

ListenerEpoll::ListenerEpoll()
{
	_epollFd = 0;
	_currNetId = 0;
	_listenFd = INVALID_SOCKET;
}

ListenerEpoll::~ListenerEpoll()
{
}

bool ListenerEpoll::init()
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

void ListenerEpoll::listen()
{
	_log->printfLog(LogType_Normal, "[net] start listen! ip = %s port = %d", _ip, _port);
	while ( !_closed )
	{
		Utils::sleep(5);
		workerThread();
	}
}

bool ListenerEpoll::shutDown()
{
	_closed = true;
	return true;
}

bool ListenerEpoll::disconnectByNetId(int netId)
{
	freeAcceptor(getAcceptor(netId));

	return true;
}

void ListenerEpoll::disconnectAll()
{
	for (int i = 0; i < MAX_CONNECT_NUM; ++i)
	{
		if (_arrAcceptor[i] != NULL)
		{
			disconnectByNetId(_arrAcceptor[i]->fd);
		}
	}
}

char * ListenerEpoll::getLocalIp()
{
	return NULL;
}
void ListenerEpoll::dispose()
{
	if (!_isInit) return;
	disconnectAll();
	SocketAPI::closeSocket(_listenFd);
	_listenFd = INVALID_SOCKET;
	::close(_epollFd);
	_epollFd = 0;
	
	SAFE_DEL_ARR(_events);
	_acceptorPool.clear();
	_netMessageQueue.clear();
	_netMessagePool.clear();
	_currNetId = 0;

	_log->printfLog(LogType_Normal, "[net] server shutDown...");
	Listener::dispose();
}

bool ListenerEpoll::startup()
{
	if ( !setupListenSocket() ) return false;

	if ( !setupEpollLib() ) return false;

	if ( !initBaseData() ) return false;

	return true;
}

bool ListenerEpoll::setupEpollLib()
{
	_epollFd = epoll_create(MAX_CONNECT_NUM);

	if (_epollFd <= 0)
	{
		_log->printfLog(LogType_Error, "[net] create epoll failed");
		return false;
	}

	_events = new epoll_event[MAX_CONNECT_NUM];

	if ( EpollAPI::epoll_add(_epollFd, _listenFd, EPOLLIN | EPOLLET) < 0 )
	{
		_log->printfLog(LogType_Error, "[net] epoll_add failed on setupEpollLib");
		return false;
	}

	return true;
}

bool ListenerEpoll::setupListenSocket()
{
	//设置每个进程允许打开的最大文件数
	struct rlimit rt;
	rt.rlim_max = rt.rlim_cur = MAX_CONNECT_NUM + 1000;

	if (setrlimit(RLIMIT_NOFILE, &rt) == -1)
	{
		_log->printfLog(LogType_Error, "[net] setrlimit failed on setupListenSocket");
		return false;
	}

	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	SocketAPI::setNonblocking(_listenFd, true);
	SocketAPI::setReuseAddr(_listenFd, true);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(_ip);
	addr.sin_port = htons(_port);


	if ( ::bind(_listenFd, (const sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR )
	{
		_log->printfLog(LogType_Error, "[net] bind failed on setupListenSocket");
		return false;
	}

	if ( ::listen(_listenFd, MAX_LISTEN_NUM) == SOCKET_ERROR )
	{
		_log->printfLog(LogType_Error, "[net] listen failed on setupListenSocket");
		return false;
	}

	return true;
}

bool ListenerEpoll::initBaseData()
{
	_acceptorPool.init(1000,500,true,MAX_CONNECT_NUM);
	_netMessageQueue.init(DEFAULT_NETMESSAGE_MAX_SERVER, 0, true);
	_netMessagePool.init(DEFAULT_NETMESSAGE_MAX_SERVER, 0, true);

	for (int i = 0; i < MAX_CONNECT_NUM; ++i)
		_arrAcceptor[i] = NULL;

	return true;
}

void ListenerEpoll::workerThread()
{
	int nfds = epoll_wait(_epollFd, _events, MAX_CONNECT_NUM, -1);

	if (nfds == -1) 
	{
		_log->printfLog(LogType_Warning, "[net] epoll_wait timeout on workerThread");
		return;
	}

	for (int i = 0; i < nfds; ++i)
	{
		epoll_event * pEvent = &_events[i];
		if (pEvent->data.fd == _listenFd)
		{
			//ET只会触发一次accept 需要循环调用
			while ( onAccept()){}
			//onAccept();
		}
		else
		{
			Acceptor * pAcceptor = (Acceptor*)pEvent->data.ptr;
			if(!pAcceptor)
			{
				_log->printfLog(LogType_Warning, "[net] pEvent->data.ptr is NULL on workerThread");
				continue;
			}

			if (pEvent->events & (EPOLLERR | EPOLLPRI))
			{
				disconnectSocket(pAcceptor);
				continue;
			}

			if (pEvent->events & EPOLLIN)
			{
				if (!onRead(pAcceptor))
					disconnectSocket(pAcceptor);
			}

			if (pEvent->events & EPOLLOUT)
			{
				if (!onWrite(pAcceptor, true))
					disconnectSocket(pAcceptor);
			}
		}
	}
}

bool ListenerEpoll::onAccept()
{
	struct sockaddr_in addr;
	unsigned int size = sizeof(addr);
	SOCKET fd = accept(_listenFd, (sockaddr *)&addr, &size);
	
	if (fd <= SOCKET_ERROR)
	{
		//表示所有连接都处理完 或者 资源已满要等待
		if( (fd == SOCKET_ERROR) && (errno == EAGAIN || errno == EWOULDBLOCK) )
			return false;
		
		_log->printfLog(LogType_Error, "[net] accept failed on onAccept errno = %d", errno);
		return false;
	}

	Acceptor* pAcceptor = newAcceptor();
	if ( !pAcceptor )
	{
		_log->printfLog(LogType_Error, "[net] acceptor pool is full on onAccept");
		SocketAPI::closeSocket(fd);
		return false;
	}
	pAcceptor->fd = fd;
	SocketAPI::setNonblocking(pAcceptor->fd, true);
	SocketAPI::setLinger(pAcceptor->fd, true);
	SocketAPI::setSendBuffSize(pAcceptor->fd, MAX_BUFFER_LEN);
	SocketAPI::setRecvBuffSize(pAcceptor->fd, MAX_BUFFER_LEN);
	//memcpy( pAcceptor->ip, inet_ntoa(addr.sin_addr), sizeof(addr.sin_addr));
	strcpy(pAcceptor->ip, inet_ntoa(addr.sin_addr));
	pAcceptor->port = htons(addr.sin_port);
	EpollAPI::epoll_add(_epollFd, pAcceptor->fd, EPOLLIN | EPOLLOUT | EPOLLET | EPOLLERR | EPOLLPRI, pAcceptor);
	_log->printfLog(LogType_Normal, "[net] client connect success nedId = %d, ip = %s:%d ", pAcceptor->netId, pAcceptor->ip, pAcceptor->port);

	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->netId = pAcceptor->netId;
	pMsg->state = NetState_Connect;
	_netMessageQueue.push(pMsg);
	return true;
}

bool ListenerEpoll::onRead(Acceptor * pAcceptor)
{
	memset(_cacheBuff,0, MAX_BUFFER_LEN);
	int offset = 0;
	//ET只会触发一次EPOLLIN 需要循环调用把所有数据都读取完成, 因为ET模式触发条件是缓冲区从空到有数据才出发， 不读为空则永远不会出发
	while(true)
	{
		int ret = ::recv(pAcceptor->fd, _cacheBuff + offset, MAX_BUFFER_LEN, 0);
		if (ret <= SOCKET_ERROR)
		{
			if ( (ret == SOCKET_ERROR) && (errno == EAGAIN || errno == EWOULDBLOCK ) )
				break;

			_log->printfLog(LogType_Error, "[net] recv failed fd = %d  on onRead errno = %d", pAcceptor->fd,errno);
			return false;
		}
		else if (ret == 0 )
		{
			return false;
		}

		offset += ret;
	}

	bool bol = pAcceptor->readBuff.write(_cacheBuff, offset);
	if (!bol)
	{
		_log->printfLog(LogType_Warning, "[net] readIO is full on onRead ");
		return false;
	}

	char *p = NULL;
	int len = 0;
	while(BuffParserUtil::getDataPacket(pAcceptor->readBuff, p, len))
	{
		NetMessage * pMsg = _netMessagePool.newObj();
		if (!pMsg)
		{
			_log->printfLog(LogType_Warning, "[net] netMessagePool is full on onRead ");
			return false;
		}

		pMsg->netId = pAcceptor->netId;
		pMsg->state = NetState_Receive;
		pMsg->data = p;
		pMsg->len = len;
		_netMessageQueue.push(pMsg);
	}

	return true;
}

bool ListenerEpoll::onWrite(Acceptor * pAcceptor, bool isChange)
{
	int len = pAcceptor->writeBuff.getUsedSize();
	pAcceptor->writeBuff.copy(_cacheBuff, MAX_BUFFER_LEN, len);
	int offset = 0;
	//需要循环调用把所有数据都发送完成, 因为ET模式触发条件是缓冲满到非满才触发
	while(len > 0)
	{
		int ret = ::send(pAcceptor->fd, _cacheBuff + offset, len, MSG_NOSIGNAL);
		if (ret <= SOCKET_ERROR)
		{
			if ( (ret == SOCKET_ERROR) && (errno == EAGAIN || errno == EWOULDBLOCK ))
			{
				pAcceptor->writeBuff.remove(ret);
				//EpollAPI::epoll_change(_epollFd ,pAcceptor->fd, EPOLLIN | EPOLLOUT | EPOLLET | EPOLLERR | EPOLLPRI, pAcceptor);
				return true;
			}

			_log->printfLog(LogType_Error, "[net] send failed fd = %d  on onWrite errno = %d", pAcceptor->fd, errno);
			return false;
		}
		len -= ret;
		offset += ret;
		pAcceptor->writeBuff.remove(ret);
	}

	//if (isChange)
		//EpollAPI::epoll_change(_epollFd ,pAcceptor->fd, EPOLLIN | EPOLLOUT | EPOLLET | EPOLLERR | EPOLLPRI, pAcceptor);
	pAcceptor->isSend = false;
	return true;
}

bool ListenerEpoll::disconnectSocket(Acceptor * pAcceptor)
{
	SAFE_RET_VAL(pAcceptor->fd, INVALID_SOCKET, false);
	_log->printfLog(LogType_Normal, "client disconnect netId = %d ! ip = %s", pAcceptor->netId, pAcceptor->ip);

	if (pAcceptor->isNetmessage)
	{
		NetMessage * pMsg = _netMessagePool.newObj();
		pMsg->netId = pAcceptor->netId;
		pMsg->state = NetState_Close;
		_netMessageQueue.push(pMsg);
	}

	EpollAPI::epoll_del(_epollFd, pAcceptor->fd, EPOLLIN | EPOLLOUT | EPOLLET | EPOLLERR | EPOLLPRI, NULL);

	freeAcceptor(pAcceptor);
	return true;
}

bool ListenerEpoll::getNetData(NetData & netData)
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

void ListenerEpoll::sendData(int netId, char * data, int len)
{
	SAFE_RET(_closed, true);
	SAFE_RET(_isInit, false);
	Acceptor * pAcceptor = getAcceptor(netId);

	if ( !pAcceptor )
	{
		_log->printfLog(LogType_Warning, "[net] netId = %d not exist", netId);
		return;
	}

	if ( MAX_PACKET_SIZE < len + BUFFER_HEAD_LEN )
	{
		_log->printfLog(LogType_Warning, "[net] netId = %d send data len > MAX_PACKET_SIZE", netId);
		return;
	}

	SAFE_RET(pAcceptor->closed, true);

	if (!BuffParserUtil::writeDataPacket( pAcceptor->writeBuff, data, len))
	{
		_log->printfLog(LogType_Warning, "[net] writeIO is full on sendData ");
		disconnectSocket(pAcceptor);
	}

	if (!pAcceptor->isSend)
	{
		pAcceptor->isSend = true;
		onWrite(pAcceptor, false);
	}
}

Acceptor * ListenerEpoll::getAcceptor(int netId)
{
	if (netId < 0 || netId >= MAX_CONNECT_NUM)
		return NULL;

	return _arrAcceptor[netId];
}

Acceptor* ListenerEpoll::newAcceptor()
{
	CREATE_MUTEX(&_acceptorLock);
	Acceptor* ptr = _acceptorPool.newObj();

	if ( ptr == NULL )
	{
		_log->printfLog(LogType_Error, "[net] SocketContext Pool full in newSocketContext");
		return NULL;
	}

	while (true)
	{
		if (_currNetId >= MAX_CONNECT_NUM)
			_currNetId = 0;

		if ( _arrAcceptor[_currNetId] == NULL )
		{
			_arrAcceptor[_currNetId] = ptr;
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

void ListenerEpoll::freeAcceptor(Acceptor * pAcceptor)
{
	SAFE_RET( pAcceptor, NULL);
	CREATE_MUTEX(&_acceptorLock);

	if (pAcceptor->netId >= 0 && pAcceptor->netId < MAX_CONNECT_NUM)
		_arrAcceptor[pAcceptor->netId] = NULL;

	_acceptorPool.releaseObj(pAcceptor);
} 

#endif