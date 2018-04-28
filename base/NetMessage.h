/************************************************************************/
/*							  接收网络数据消息包							*/
/************************************************************************/
#ifndef __NETMESSGE_H__
#define __NETMESSGE_H__
#include "NetDefine.h"
#include "CommonDefine.h"
#include "PoolObj.h"

class BuffParser;
class NetMessage : public PoolObj
{
public:
	NetMessage() {clear();};
	~NetMessage() {dispose(); clear();};

	void initByPool() {clear();};
	void releaseByPool() {dispose(); clear(); };
	Int netId;
	NetState state;
	char * data;
	Int len;

	void clear()
	{
		data = NULL;
		netId = 0;
		state = NetState_Null;
		len = 0;
	}

	void dispose()
	{
		SAFE_DEL_ARR(data);
	}
};


#endif