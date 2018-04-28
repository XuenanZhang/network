#if defined(_LINUX)
#include "EpollAPI.h"
#include <sys/epoll.h>

namespace EpollAPI
{
	int epoll_add(int epollfd, int fd, int newMode, void * ptr)
	{
		epoll_event event;
		event.data.fd = fd;
		event.events = newMode;
		if (ptr) event.data.ptr = ptr;
		//if (oneshot) event.events |= EPOLLONESHOT;
		//setnonblocking(fd);

		return epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	}

	int epoll_change(int epollfd, int fd, int newMode, void * ptr)
	{
		epoll_event event;
		event.data.fd = fd;
		event.events = newMode;
		if (ptr) event.data.ptr = ptr;

		return epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
	}

	int epoll_del(int epollfd, int fd)
	{
		epoll_event event;
		event.data.fd = fd;
		event.data.prt = NULL;

		return epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &event);
	}

	int reset_oneshot(int epollfd, int fd)
	{
		return epoll_add(epollfd, fd, EPOLLIN | EPOLLET | EPOLLONESHOT);
	}

}

#endif