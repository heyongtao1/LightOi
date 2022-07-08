#ifndef _UTIL_H
#define _UTIL_H
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <strings.h>
#include <fcntl.h>
#include <arpa/inet.h>
namespace epoll_util{
	//----------------------------------------
	static int setnonblocking(int fd)
	{
		int old_opt = fcntl(fd,F_GETFL);
		int new_opt = old_opt | O_NONBLOCK;
		fcntl(fd,F_SETFL,new_opt);
		return old_opt;
	}
	static void addfd(int epollfd,int fd, bool one_shot)
	{
		epoll_event event;
		event.data.fd = fd;
		event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
		if(one_shot){
			event.events |= EPOLLONESHOT;
		}
		epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
		setnonblocking(fd);
	}
	
	static void removefd(int epollfd,int fd)
	{
		epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,0);
	}

	static void modfd(int epollfd,int fd,int ev)
	{
		epoll_event event;
		event.data.fd = fd;
		event.events = EPOLLONESHOT | EPOLLET | EPOLLRDHUP | ev;
		epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
	}
	static char* IP_tostring(struct sockaddr_in& clientAddr)
	{
		return inet_ntoa(clientAddr.sin_addr);
	}
	//--------------------------------------
}

#endif