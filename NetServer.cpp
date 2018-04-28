
#include "NetServer.h"
#include "NetStream.h"
#if defined(_WINDOWS)
#include "ListenerIOCP.h"
#include "ListenerLibEvent.h"	
#elif defined(_LINUX)
#include "ListenerEpoll.h"
#endif

NetServer::NetServer()
{
	_listener = NULL;
	_log = Log::Ptr();
	_isInit = false;
	_port = 0;
	_rModel = ReceiveModel_Pack;
}

NetServer::~NetServer()
{
	SAFE_DEL(_listener);
}
bool NetServer::init()
{
	if (_isInit)
	{
		_log->printfLog(LogType_Error, "NetServer repeat init !");
		return false;
	}

	if (_listener)
	{
		_log->printfLog(LogType_Error, "listener is exist!");
		return false;
	}
	
#if defined(_WINDOWS)
	//_listener = new ListenerLibEvent();
	_listener = new ListenerIOCP();
#elif defined(_LINUX)
	_listener = new ListenerEpoll();
#endif

	if ( !_listener->init() )
		return false;

	_isInit = true;
	return true;
}

bool NetServer::shutDown()
{
	SAFE_RET_VAL(_listener, NULL, false);
	_listener->shutDown();
	return true;
}

void NetServer::setIP( const String &strIP )
{
	_listener->setIP(strIP);
}
void NetServer::setIP( const char * strIP )
{
	_listener->setIP(strIP);
}

const char * NetServer::getIP() 
{ 
	return _listener->getIP(); 
}

void NetServer::run()
{
	_listener->listen();
}

void NetServer::onExit()
{
	if (_listener)
		_listener->dispose();
	
	_log->printfLog(LogType_Normal, "[net] net thread exit!");
	_isInit = false;
}

bool NetServer::disconnectByNetId(int netId)
{
	SAFE_RET_VAL(_listener, NULL, false);
	return _listener->disconnectByNetId(netId);
}

void NetServer::disconnectAll()
{
	_listener->disconnectAll(); 
}

char * NetServer::getLocalIp()
{
	return _listener->getLocalIp();
}

bool NetServer::getNetData(NetData & netData)
{
	SAFE_RET_VAL(_listener, NULL, false);
	return _listener->getNetData(netData);
}

void NetServer::sendData(int netId, char * data, int len)
{
	SAFE_RET(_listener, NULL);
	_listener->sendData(netId, data, len);
}