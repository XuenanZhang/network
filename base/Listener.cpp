#include "Listener.h"
#include "NetDefine.h"


Listener::Listener()
{
	_isInit = false;
	_closed = false;
	memset(_ip, 0, 32);
	setIP(DEFAULT_IP);
	setPort(DEFAULT_PORT);
	setReceiveModel(DEFAULT_REVEIVEMODEL);
	_log = Log::Ptr();
}

Listener::~Listener()
{
}

void Listener::dispose()
{
	_isInit = false;
	_closed = false;
}