#include "Connector.h"
#include "NetDefine.h"
#include "log.h"

Connector::Connector()
{
	_isInit = false;
	_isConncet = false;
	_reconnect = false;
	memset(_ip, 0, 32);
	setIP(DEFAULT_IP);
	setPort(DEFAULT_PORT);
	setReceiveModel(DEFAULT_REVEIVEMODEL);
	_log = Log::Ptr();
}

Connector::~Connector()
{

}