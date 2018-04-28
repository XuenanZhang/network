#ifndef _EPOLLAPI_H_
#define _EPOLLAPI_H_

#if defined(_LINUX)
#include "CommonDefine.h"
namespace EpollAPI
{
	int epoll_add(int epollfd, int fd, int newMode, void * ptr = NULL);
	int epoll_change(int epollfd, int fd, int newMode, void * ptr = NULL);
	int epoll_del(int epollfd, int fd);
	int reset_oneshot(int epollfd, int fd);
}
#endif

#endif