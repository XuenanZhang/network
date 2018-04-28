#include "NetClient.h"
#if defined(_WINDOWS)
#include "ConnectorLibEvent.h"
#include "ConnectorIOCP.h"
#elif defined(_LINUX)
#include "ConnectorEpoll.h"
#endif

NetClient::NetClient()
{
	_connector = NULL;
	_log = Log::Ptr();
	_isInit = false;
	_port = 0;
	_rModel = ReceiveModel_Pack;
}

NetClient::~NetClient()
{
	SAFE_DEL(_connector);
}

bool NetClient::init()
{
	if (_isInit)
	{
		_log->printfLog(LogType_Error, "NetClient repeat init !");
		return false;
	}

	if (_connector)
	{
		_log->printfLog(LogType_Error, "connector is exist!");
		return false;
	}
	//_connector = new ConnectorLibEvent();
#if defined(_WINDOWS)
	_connector = new ConnectorIOCP();
#elif defined(_LINUX)
	_connector = new ConnectorEpoll();
#endif

	if ( !_connector->init() )
		return false;
	
	_log->printfLog(LogType_Normal, "connector init complete !");
	_isInit = true;
	return true;
}

bool NetClient::disconnect()
{
	_connector->disconnect();
	return true;
}

bool NetClient::isConnecting()
{
	SAFE_RET_VAL(_connector, NULL, false);
	return _connector->isConnecting();
}

bool NetClient::reconnect()
{
	SAFE_RET_VAL(_connector, NULL, false);
	return _connector->reconnect();
}


void NetClient::setIP( const String &strIP )
{
	_connector->setIP(strIP);
}
void NetClient::setIP( const char * strIP )
{
	_connector->setIP(strIP);
}

const char * NetClient::getIP() 
{ 
	return _connector->getIP(); 
}

void NetClient::run()
{
	_connector->connect();
}

void NetClient::onExit()
{
	if (_connector)
		_connector->dispose();

	_log->printfLog(LogType_Normal, "[net] net thread exit!");
	_isInit = false;
}

bool NetClient::getNetData(NetData & netData)
{
	SAFE_RET_VAL(_connector, NULL, false);
	return _connector->getNetData(netData);
}

void NetClient::sendData(char * data, int len)
{
	SAFE_RET(_connector, NULL);
	_connector->sendData(data, len);
}