#include "SocketAPI.h"
#if defined(_WINDOWS)
#include <WinSock2.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>	
#endif

namespace SocketAPI
{
	bool setSockOpt(SOCKET socket, int level, int optname, const void * optval, unsigned int optlen) 
	{
#if defined(_WINDOWS)
		return ::setsockopt(socket, level, optname, (char*)optval, (int)optlen) == SOCKET_ERROR ? false : true;
#elif defined(_LINUX)
		return ::setsockopt(socket, level, optname, optval, (socklen_t)optlen)  == SOCKET_ERROR ? false : true;
#endif
		return false;
	}

	bool closeSocket (SOCKET s)
	{
#if defined(_WINDOWS)
		::closesocket(s);
		return true;
#elif defined(_LINUX)
		::shutdown(s, SHUT_RDWR);
		::close(s);
		return false;
#endif
		return false;
	}

	bool setNonblocking (SOCKET s,  bool onff)
	{
#if defined(_WINDOWS)
		return ioctlsocket(s, FIONBIO, (unsigned long*)&onff) == SOCKET_ERROR ? false : true;
#elif defined(_LINUX)
		int opts = fcntl (s, F_GETFL, 0);

		if (opts < 0) return false;

		if (onff)
			opts = (opts | O_NONBLOCK);
		else
			opts = (opts & ~O_NONBLOCK);

		fcntl(s, F_SETFL, opts);
		return opts < 0 ? false : true;
#endif
		return false;
	}

	bool setLinger (SOCKET s, bool onff , unsigned int lingerTime) 
	{
		//onff = 0, linger忽略 close()立刻返回，底层将未发送完的数据发送完成后再释放资源，优雅退出
		//onff != 0, linger = 0 close()立刻返回，但不会发送未发完的数据，用过REST包强制关闭socket， 强制退出
		//onff != 0, linger > 0 close()不会立刻返回，延迟linger时间，如果时间内发完 为上面1，否则为2

#if defined(_WINDOWS)
		LINGER linger;
		linger.l_onoff = int(onff);
		linger.l_linger = lingerTime;
		return setSockOpt(s, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
#elif defined(_LINUX)
		linger linger;
		linger.l_onoff = int(onff);
		linger.l_linger = lingerTime;
		return setSockOpt(s, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
#endif
		return false;
	}

	bool setReuseAddr (SOCKET s, bool onff) 
	{
		return setSockOpt(s, SOL_SOCKET, SO_REUSEADDR, &onff, sizeof(bool));
	}

	bool setSendBuffSize (SOCKET s, unsigned int size)
	{
		return setSockOpt(s, SOL_SOCKET, SO_SNDBUF, &size, sizeof(unsigned int));
	}

	bool setRecvBuffSize (SOCKET s, unsigned int size)
	{
		return setSockOpt(s, SOL_SOCKET, SO_RCVBUF, &size, sizeof(unsigned int));
	}
}