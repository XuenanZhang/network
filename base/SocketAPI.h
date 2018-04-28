#ifndef __SOCKETAPI_H__
#define __SOCKETAPI_H__

#if defined(_WINDOWS)
#include <WinSock2.h>
#elif defined(_LINUX)
typedef		int		SOCKET;
#define     INVALID_SOCKET   -1
#define		SOCKET_ERROR	 -1
#endif

namespace SocketAPI 
{
	/** 设置socke操作 **/
	bool setSockOpt(SOCKET socket, int level, int optname, const void * optval, unsigned int optlen);

	/** 关闭socket **/
	bool closeSocket (SOCKET s);

	/** 是否非阻塞 **/
	bool setNonblocking (SOCKET s, bool onff = true); 
	
	/** 退出方式 **/
	bool setLinger (SOCKET s, bool onff , unsigned int lingerTime = 0); 

	/** 关闭后是否重用 **/
	bool setReuseAddr (SOCKET s, bool onff);

	/** 发送buff大小 **/
	bool setSendBuffSize (SOCKET s, unsigned int size);

	/** 接收buff大小 **/
	bool setRecvBuffSize (SOCKET s, unsigned int size);
}






#endif