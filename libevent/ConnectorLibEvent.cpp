#include "ConnectorLibEvent.h"
#include "BuffParser.h"
#include "BuffParserPack.h"
#include "NetMessage.h"
#include <event2/event.h>
#include <event2/bufferevent.h>
#include "log.h"

static void loop_event_cb(evutil_socket_t, short, void *ctx)
{
	ConnectorLibEvent *pConnector = static_cast<ConnectorLibEvent *>(ctx);
	pConnector->onTick();
}

static void connect_cb(struct bufferevent *bev, short events, void *ctx)
{
	ConnectorLibEvent *pConnector = static_cast<ConnectorLibEvent *>(ctx);
	if (events & BEV_EVENT_CONNECTED)
		pConnector->onConnect(true);
	else
		pConnector->onConnect(false);
}

static void buffParser_read_compete( BuffParser * pBuff, void *ptr)
{
	ConnectorLibEvent *pConnector = static_cast<ConnectorLibEvent *>(ptr);
	pConnector->onReadComplete();
}

static void buffParser_Fail( BuffParser * pBuff, void *ptr)
{
	ConnectorLibEvent *pConnector = static_cast<ConnectorLibEvent *>(ptr);
	pConnector->onReadFail();
}

ConnectorLibEvent::ConnectorLibEvent()
{
	_isInit = false;
	_reconnect = false;
	_evBase = NULL;
	_loopEvent = NULL;
	_buffevent = NULL;
	_buffParser = NULL;
}

ConnectorLibEvent::~ConnectorLibEvent()
{

}

bool ConnectorLibEvent::init()
{
	if (_isInit)
	{
		_log->printfLog(LogType_Error, "[net] client had init complete");
		return false;
	}

	_log->printfLog(LogType_Normal, "[net] start startup client...");

	if (!startup())
	{
		_log->printfLog(LogType_Error, "[net] client startup fail !");
		return false;
	}
	_isInit = true;

	_log->printfLog(LogType_Normal, "[net] init client success !!!");
	return true;
}

bool ConnectorLibEvent::connect()
{
	/*if (isConnecting())
	{
	_log->printfLog(LogType_Warning, "[net] repeat connect, client is connecting");
	return false;
	}*/

	int err = event_base_dispatch(_evBase);

	return true;
}

void ConnectorLibEvent::disconnect()
{
	if (!isConnecting()) return;
	_isConncet = false;
	event_base_loopexit(_evBase, NULL);
}

bool ConnectorLibEvent::reconnect()
{
	if (_reconnect) return false;
	_reconnect = true;
	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->state = NetState_Close;
	_opQueue.push(pMsg);

	return true;
}

void ConnectorLibEvent::dispose()
{
	if (!_isInit) return;
	_isInit = false;
	onTick();
	SAFE_DEL(_buffParser);
	_buffevent = NULL;
	event_free(_loopEvent);
	_loopEvent = NULL;
	event_base_free(_evBase);
	_evBase = NULL;

	_netMessageQueue.clear();
	_netMessagePool.clear();
	_opQueue.clear();
}

bool ConnectorLibEvent::getNetData(NetData & netData)
{
	if (!_isInit) return false;
	if (_netMsgCache)
	{
		_opQueue.push(_netMsgCache);
		_netMsgCache = NULL;
	}

	NetMessage * pMsg = _netMessageQueue.pop();
	if (!pMsg) return false;
	netData.state = pMsg->state;
	netData.data = pMsg->data;
	netData.len = pMsg->len;
	_netMsgCache = pMsg;

	return true;
}

void ConnectorLibEvent::sendData(char * data, int len)
{
	if (!_isInit) return;
	//此处效率有问题，需优化，不能new char
	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->state = NetState_Send;
	char * pChar = new char[len + 1];
	pChar[len] = 0;
	memcpy(pChar, data, len);
	pMsg->data = pChar;
	pMsg->len = len;
	_opQueue.push(pMsg);
}

bool ConnectorLibEvent::startup()
{
	if ( !loadSocketLib() ) return false;

	if ( !loadLibEvent() ) return false;

	if ( !initBaseData() ) return false;

	if ( !setupConnector() ) return false;

	return true;
}

bool ConnectorLibEvent::initBaseData()
{
	_netMessageQueue.init(DEFAULT_NETMESSAGE_MAX_CLIENT);
	_netMessagePool.init(DEFAULT_NETMESSAGE_MAX_CLIENT, DEFAULT_NETMESSAGE_MAX_CLIENT, true);
	_opQueue.init(DEFAULT_NETMESSAGE_MAX_CLIENT);

	_netMsgCache = NULL;
	return true;
}

bool ConnectorLibEvent::loadSocketLib()
{
#ifdef _WINDOWS
	WSADATA wsaData;
	int ret;
	ret = WSAStartup(MAKEWORD(2,2), &wsaData); //请求winsock库
	if (NO_ERROR != ret)
	{
		_log->printfLog(LogType_Error, "[net] request SocketLib fail in LoadSocketLib ");
		return false; 
	}
#elif defined(_LINUX)

#endif

	return true;
}

bool ConnectorLibEvent::loadLibEvent()
{
	_evBase = event_base_new();
	const char * pMod = event_base_get_method(_evBase);
	if (!_evBase)
	{
		event_base_free(_evBase);
		_log->printfLog(LogType_Error, "[net] event_base_new fail: %s",getErrorToStr());
		return false;
	}

	_loopEvent = event_new(_evBase, -1, EV_PERSIST, loop_event_cb, this);
	timeval t = {0, FRAME_RATE * 100};
	if ( -1 == event_add(_loopEvent, &t) )
	{
		event_free(_loopEvent);
		event_base_free(_evBase);
		_log->printfLog(LogType_Error, "[net] loopEvent event_new fail: %s",getErrorToStr());
		return false;
	}

	return true;
}

bool ConnectorLibEvent::setupConnector()
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(_ip);
	sin.sin_port = htons(_port);

	_buffevent = bufferevent_socket_new(_evBase, -1, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(_buffevent, NULL, NULL, connect_cb, this);

	if (bufferevent_socket_connect(_buffevent, (struct sockaddr *)&sin, sizeof(sin)) == 0)
	{
		_isConncet = true;
		_log->printfLog(LogType_Normal, "[net] client connect success !!!");
	}
	else
	{
		_log->printfLog(LogType_Error, "[net] bufferevent_socket_connect fail: %s",getErrorToStr());
		bufferevent_free(_buffevent);
		_buffevent = NULL;
		_isConncet = false;
		return false;
	}

	return true;
}

void ConnectorLibEvent::doReconnect()
{
	SAFE_DEL(_buffParser);
	NetMessage * pMsg;
	while ( pMsg = _opQueue.pop())
		_netMessagePool.releaseObj(pMsg);
	while ( pMsg = _netMessageQueue.pop())
		_netMessagePool.releaseObj(pMsg);
	setupConnector();
	_reconnect = false;
}


void ConnectorLibEvent::processQueue()
{
	NetMessage * pMsg = NULL;
	while (true)
	{
		pMsg = _opQueue.pop();
		if ( !pMsg ) break;

		if (pMsg->state == NetState_Close )
		{
			if (_reconnect)
				doReconnect();
			else
				disconnect();
		}
		else if (pMsg->state == NetState_Send )
		{
			if ( _buffParser && false == _buffParser->sendData(pMsg->data, pMsg->len) )
				_log->printfLog(LogType_Error, "[net] write buffevent data fail !");
		}

		_netMessagePool.releaseObj(pMsg);
	}
}

void ConnectorLibEvent::onTick()
{
	processQueue();
}

void ConnectorLibEvent::onConnect(bool bRes)
{
	if (bRes)
	{
		switch (_rModel)
		{
		case ReceiveModel_Push:
			break;
		case ReceiveModel_Pull:
			break;
		case ReceiveModel_Pack:
			_buffParser = new BuffParserPack();
			_buffParser->init(_buffevent, this, buffParser_read_compete, buffParser_Fail);
			NetMessage * pMsg = _netMessagePool.newObj();
			pMsg->state = NetState_Connect;
			_netMessageQueue.push(pMsg);
			break;
		}
	}
	else
	{
		disconnect();
		_log->printfLog(LogType_Normal, "[net] client connect !!!");
	}
}

void ConnectorLibEvent::onReadComplete()
{
	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->state = NetState_Receive;
	switch (_rModel)
	{
	case ReceiveModel_Push:
		break;
	case ReceiveModel_Pull:
		break;
	case ReceiveModel_Pack:
		pMsg->data = static_cast<BuffParserPack *>(_buffParser)->getData();
		pMsg->len = static_cast<BuffParserPack *>(_buffParser)->getTotal();
		break;
	}

	_netMessageQueue.push(pMsg);
}

void ConnectorLibEvent::onReadFail()
{
	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->state = NetState_Close;
	_netMessageQueue.push(pMsg);

	_log->printfLog(LogType_Warning, "[net] read data fail : %s",getErrorToStr());
}