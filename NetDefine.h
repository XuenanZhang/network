#ifndef __NETDEFINE_H__
#define __NETDEFINE_H__

//字节序
#define NET_BYTE_INT( i ) ((i&0xFF000000)>>24 | (i&0x00FF0000)>>8 | (i&0x0000FF00)<<8 | (i&0x000000FF)<<24)
#define NET_BYTE_SHORT( i ) ((i&0xFF00)>>8 | (i&0x00FF)<<8)

/** 接收模型 **/
enum ReceiveModel
{
	ReceiveModel_Push, //直接推送
	ReceiveModel_Pull, //手动拉取
	ReceiveModel_Pack, //整包
};

/** 接收模型 **/
enum NetState
{
	NetState_Null,		//未初始化
	NetState_Connect,   //新的连接
	NetState_Close,		//关闭连接
	NetState_Receive,   //接收数据
	NetState_Send,		//发送数据
	NetState_ReConnect, //重新连接
};

struct NetData
{
	int netId;
	NetState state;
	char * data;
	int len;

	NetData():netId(0), state(NetState_Null), data(0),len(0){};
};


/** 默认端口 */
#define DEFAULT_PORT					12345    
/** 默认IP地址 */
#define DEFAULT_IP						"127.0.0.1"
//#define DEFAULT_IP					"192.168.229.200"
/** 最大socket连接数 */
#define MAX_CONNECT_NUM					2000
/** 最大listen完成队列(accept 之前） */
#define MAX_LISTEN_NUM					2000
/** 缓冲区长度 (1024*8) */
#define MAX_BUFFER_LEN					8192
/** 单个数据包最大值 (1024) */
#define MAX_PACKET_SIZE					1024  
/** 创建buff char数组最小值 */
#define BUFFER_ARR_SIZE_MIN				8
/** buff头标题长度 */
#define BUFFER_HEAD_LEN					4  
/** 默认接收模型方式 */
#define DEFAULT_REVEIVEMODEL			ReceiveModel_Pack    
/** netmessage队列最大值（包数）服 */
#define DEFAULT_NETMESSAGE_MAX_SERVER	10000
/** netmessage队列最大值（包数）客 */
#define DEFAULT_NETMESSAGE_MAX_CLIENT	1000


#endif