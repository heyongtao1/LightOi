/**
 * @file Reactor.c
 * @author HYT
 * @date 2021.11.17
 * @brief MainReactor and SubReactor inherit Reactor
 * MainReactor : Only responsible monitor for new connection events
 * SubReactor  : Only responsible monitor for connected active events
 */
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <strings.h>
#include<arpa/inet.h>
#include "Reactor.h"
namespace LightOi
{
	//----------------------------------------
	int setnonblocking(int fd)
	{
		int old_opt = fcntl(fd,F_GETFL);
		int new_opt = old_opt | O_NONBLOCK;
		fcntl(fd,F_SETFL,new_opt);
		return old_opt;
	}
	void addfd(int epollfd,int fd, bool one_shot)
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
	
	void removefd(int epollfd,int fd)
	{
		epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,0);
		close(fd);
	}

	void modfd(int epollfd,int fd,int ev)
	{
		epoll_event event;
		event.data.fd = fd;
		event.events = EPOLLONESHOT | EPOLLET | EPOLLRDHUP | ev;
		epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
	}
	//--------------------------------------
	void Reactor::EPollstart()
	{
		epfd = epoll_create(5);
		assert(epfd != -1);
	}
	
	void Reactor::EPollstop()
	{
		close(epfd);
	}
	
	void Reactor::stopLoop()
	{
		quit = true;
	}
	
	int MainReactor::listens()
	{
		//TcpSocket socket{};
		//bool success = socket.listen(_address);
		
		struct sockaddr_in address;
		bzero(&address, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_port = htons(_port);
		address.sin_addr.s_addr = htons(INADDR_ANY);

		int listenfd = socket(AF_INET,SOCK_STREAM,0);
		assert(listenfd >= 0);
		struct linger tmp = {1,0};
		setsockopt(listenfd,SOL_SOCKET,SO_LINGER,&tmp,sizeof(tmp));

		int ret = 0;
		ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
		assert(ret != -1);

		ret = listen(listenfd,5);
		assert(ret >= 0);
	
		//listen不能注册EPOLLONESHOT事件，否则只能处理一个客户连接
		addfd(epfd, listenfd, false);
		return listenfd;
	}
	
	void MainReactor::loop()
	{
		EPollstart();
		int listenfd = listens();
		while(!quit)
		{
			int number = epoll_wait(epfd, events, MAX_CONN_EVENT_NUMBER, -1);
			/* acceptor handle new connect , if accept new connect success 
			   to NewConnectCallback(newconfd)
			*/
			_acceptor.handleAccept(listenfd,number);
		}
	}
	
	void MainReactor::NewConnectCallback(int newconfd)
	{
		_disPatchcb(newconfd);
	}
	

}