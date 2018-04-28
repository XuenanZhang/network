#include "ListenerLibEvent.h"
#include "NetDefine.h"
#include "BuffParser.h"
#include "BuffParserPack.h"
#include "NetMessage.h"
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

static void loop_event_cb(evutil_socket_t, short, void *ctx)
{
	ListenerLibEvent *pListener = static_cast<ListenerLibEvent *>(ctx);
	pListener->onTick();
}

static void accept_conn_cb(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * address, int socklen, void *ctx)
{
	event_base * base = evconnlistener_get_base(listener);
	bufferevent * bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	//bufferevent_priority_set(bev, 2);
	ListenerLibEvent * pListener = static_cast<ListenerLibEvent *>(ctx);
	//char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	//getnameinfo(address, socklen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf));
	const char *pIP = inet_ntoa(((struct sockaddr_in *)address)->sin_addr);
	UInt port = htons(((struct sockaddr_in *)address)->sin_port);
	pListener->onAccept(bev, pIP, port);
}

static void accept_error_cb(struct evconnlistener *listener, void *ctx)
{
	ListenerLibEvent *pListener = static_cast<ListenerLibEvent *>(ctx);
	pListener->onAcceptError();
}

static void buffParser_read_compete( BuffParser * pBuff, void *ptr)
{
	ListenerLibEvent *pListener = static_cast<ListenerLibEvent *>(ptr);
	pListener->onReadComplete(pBuff);
}

static void buffParser_Fail( BuffParser * pBuff, void *ptr)
{
	ListenerLibEvent *pListener = static_cast<ListenerLibEvent *>(ptr);
	pListener->onReadFail(pBuff);
}

ListenerLibEvent::ListenerLibEvent()
{

}

ListenerLibEvent::~ListenerLibEvent()	
{
}
bool ListenerLibEvent::init()
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

void ListenerLibEvent::listen()
{
	int err = event_base_dispatch(_evBase);
}

void ListenerLibEvent::dispose()
{
	if (!_isInit) return;
	disconnectAll();
	onTick();
	event_free(_loopEvent);
	_loopEvent = NULL;
	evconnlistener_free(_evconnlistener);
	_evconnlistener = NULL;
	event_base_free(_evBase);
	_evBase = NULL;

	_buffPackPool.clear();
	_netMessageQueue.clear();
	_netMessagePool.clear();
	_opQueue.clear();
	_currNetId = 0;
	_totalNetNum = 0;

	_log->printfLog(LogType_Normal, "[net] server shutDown...");
	Listener::dispose();
}

bool ListenerLibEvent::shutDown()
{
	event_base_loopexit(_evBase, NULL);
	return true;
}

bool ListenerLibEvent::disconnectByNetId(int netId)
{
	BuffParser * pBuff = getBuffParser(netId);
	SAFE_RET_VAL(pBuff, NULL, false);
	if (pBuff->isClose()) return false;
	pBuff->close();
	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->netId = pBuff->getNetId();
	pMsg->state = NetState_Close;
	_opQueue.push(pMsg);

	return true;
}

void ListenerLibEvent::disconnectAll()
{
	for (int i = 0; i < MAX_CONNECT_NUM; ++i)
	{
		if (_arrBuffParser[i] != NULL)
		{
			disconnectByNetId(_arrBuffParser[i]->getNetId());
		}
	}
}

char * ListenerLibEvent::getLocalIp()
{
	return "";
}

bool ListenerLibEvent::getNetData(NetData & netData)
{
	if (!_isInit) return false;
	if (_netMsgCache)
	{
		_opQueue.push(_netMsgCache);
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

void ListenerLibEvent::sendData(int netId, char * data, int len)
{
	if (!_isInit) return;
	BuffParser * pBuff = getBuffParser(netId);
	SAFE_RET(pBuff, NULL);
	if (pBuff->isClose()) 
	{
		_log->printfLog(LogType_Error, "[net] BuffParser have closed, sendData data fail !");
		return;
	}

	//此处效率有问题，需优化，不能new char
	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->netId = netId;
	pMsg->state = NetState_Send;
	char * pChar = new char[len];
	memcpy(pChar, data, len);
	pMsg->data = pChar;
	pMsg->len = len;
	_opQueue.push(pMsg);
}

void ListenerLibEvent::processQueue()
{
	NetMessage * pMsg = NULL;
	while (true)
	{
		pMsg = _opQueue.pop();
		if ( !pMsg ) break;
		BuffParser * pBuff = getBuffParser(pMsg->netId);
		if ( !pBuff )
		{
			_log->printfLog(LogType_Error, "[net] getBuffParser equal to nil ! on processQueue");
			continue;
		}

		if (pMsg->state == NetState_Close )
		{
			freeBuffParser(pBuff);
		}
		else if (pMsg->state == NetState_Send )
		{
			if ( false == pBuff->sendData(pMsg->data, pMsg->len) )
				_log->printfLog(LogType_Error, "[net] write buffevent data fail !");
		}

		_netMessagePool.releaseObj(pMsg);
	}
}

void ListenerLibEvent::onTick()
{
	processQueue();
}

bool ListenerLibEvent::startup()
{
	//WSADATA wsaData;
	//int ret;
	//ret = WSAStartup(MAKEWORD(2,2), &wsaData); //请求winsock库
	if ( !loadSocketLib() ) return false;

	if ( !loadLibEvent() ) return false;

	if ( !setupListener() ) return false;

	if ( !initBaseData() ) return false;

	return true;
}

bool ListenerLibEvent::initBaseData()
{
	_buffPackPool.init(1000,500,false,MAX_CONNECT_NUM);

	for (int i = 0; i < MAX_CONNECT_NUM; ++i)
		_arrBuffParser[i] = NULL;

	_netMessageQueue.init(DEFAULT_NETMESSAGE_MAX_SERVER);
	_netMessagePool.init(DEFAULT_NETMESSAGE_MAX_SERVER, DEFAULT_NETMESSAGE_MAX_SERVER, true);
	_opQueue.init(DEFAULT_NETMESSAGE_MAX_SERVER, 0, true);

	_currNetId = 0;
	_totalNetNum = 0;
	_netMsgCache = NULL;

	return true;
}

bool ListenerLibEvent::loadSocketLib()
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

bool ListenerLibEvent::loadLibEvent()
{
	//event_config * cfg = event_config_new();
	//event_config_avoid_method(cfg, "select");
	//event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
	//event_config_require_features(cfg, EV_FEATURE_ET);
	//_evBase = event_base_new_with_config(cfg);
	//event_config_free(cfg);
	//evthread_use_windows_threads();
	_evBase = event_base_new();
	//const char * pMod = event_base_get_method(_evBase);
	//printf("%s",pMod);
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

bool ListenerLibEvent::setupListener()
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(_ip);
	sin.sin_port = htons(_port);

	_evconnlistener = evconnlistener_new_bind(_evBase, accept_conn_cb, this, 
		LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, MAX_LISTEN_NUM, (struct sockaddr*)&sin, sizeof(sin));

	if (!_evconnlistener)
	{
		event_free(_loopEvent);
		event_base_free(_evBase);
		_log->printfLog(LogType_Error, "[net] evconnlistener_new_bind fail: %s",getErrorToStr());
		return false;
	}

	evconnlistener_set_error_cb(_evconnlistener, accept_error_cb);

	return true;
}


void ListenerLibEvent::onAccept(bufferevent *pBev, const char *pIP, UInt nIP)
{
	BuffParser * pBuff = newBuffParser();

	if ( !pBuff )
		_log->printfLog(LogType_Warning, "[net] create BuffParser failed!!! on onAccept");
	else
	{
		_log->printfLog(LogType_Normal, "[net] new connect ip=%s  port=%d netId=%d", pIP, nIP, pBuff->getNetId());

		pBuff->init(pBev, this, buffParser_read_compete, buffParser_Fail);

		NetMessage * pMsg = _netMessagePool.newObj();
		pMsg->netId = pBuff->getNetId();
		pMsg->state = NetState_Connect;
		_netMessageQueue.push(pMsg);
	}
}

void ListenerLibEvent::onAcceptError()
{
	_log->printfLog(LogType_Error, "[net] accept connect fail: %s", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
}

void ListenerLibEvent::onReadComplete(BuffParser * pBuff)
{
	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->netId = pBuff->getNetId();
	pMsg->state = NetState_Receive;
	switch (_rModel)
	{
	case ReceiveModel_Push:
		break;
	case ReceiveModel_Pull:
		break;
	case ReceiveModel_Pack:
		pMsg->data = static_cast<BuffParserPack *>(pBuff)->getData();
		pMsg->len = static_cast<BuffParserPack *>(pBuff)->getTotal();
		break;
	}
	
	_netMessageQueue.push(pMsg);
}

void ListenerLibEvent::onReadFail(BuffParser * pBuff)
{
	SAFE_RET(pBuff, NULL);
	pBuff->close();
	NetMessage * pMsg = _netMessagePool.newObj();
	pMsg->netId = pBuff->getNetId();
	pMsg->state = NetState_Close;
	_netMessageQueue.push(pMsg);

	//disconnectByNetId(pMsg->netId);
	
	_log->printfLog(LogType_Warning, "[net] read data fail : %s",getErrorToStr());
}

BuffParser* ListenerLibEvent::newBuffParser()
{
	//create buff
	BuffParser * pBuff = NULL;
	switch (_rModel)
	{
	case ReceiveModel_Push:
		break;
	case ReceiveModel_Pull:
		break;
	case ReceiveModel_Pack:
		pBuff = _buffPackPool.newObj();
		break;
	default:
		_log->printfLog(LogType_Error, "[net] ReceiveModel not exist");
		return NULL;
	}

	if ( !pBuff )
	{
		_log->printfLog(LogType_Warning, "[net] Connect reached the maximum on newBuffParser");
		return NULL;
	}

	//create netId 
	// poolId不能作为netId因为会变，这里只能先这么创建netId，以后在优化. 如果pBuff能创建则肯定没超过最大值
	while (true)
	{
		if (_currNetId >= MAX_CONNECT_NUM)
			_currNetId = 0;

		if ( _arrBuffParser[_currNetId] == NULL )
		{
			_arrBuffParser[_currNetId] = pBuff;
			pBuff->setNetId(_currNetId);
			++_currNetId;
			++_totalNetNum;
			break;
		}
		else
		{
			++_currNetId;
		}
	}

	return pBuff;
}

void ListenerLibEvent::freeBuffParser(BuffParser * pBuffParser)
{
	SAFE_RET(pBuffParser, NULL);

	int netId = pBuffParser->getNetId();
	switch (_rModel)
	{
	case ReceiveModel_Push:
		break;
	case ReceiveModel_Pull:
		break;
	case ReceiveModel_Pack:
		_buffPackPool.releaseObj(static_cast<BuffParserPack *>(pBuffParser));
		break;
	default:
		return;
	}

	_arrBuffParser[netId] = NULL;
	--_totalNetNum;
}

BuffParser * ListenerLibEvent::getBuffParser(int netId)
{
	if (netId < 0 || netId >= MAX_CONNECT_NUM)
		return NULL;

	return _arrBuffParser[netId];
}
