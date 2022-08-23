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
#include <arpa/inet.h>
#include "Reactor.h"
#include "util/util.h"
namespace LightOi
{
	void Reactor::EPollstart()
	{
		epfd = epoll_create(200);
		assert(epfd != -1);
	}
	
	void Reactor::EPollstop()
	{
		close(epfd);
	}
	
	void Reactor::stop()
	{
		stopLoop();
		EPollstop();
	}
	
	void Reactor::stopLoop()
	{
		quit = true;
	}
	
	int MainReactor::listens()
	{
		serverSocket = (ServerSocketImpl*)SocketFactory::createServerSocket(_port);

		int ret = serverSocket->listen(128);
		assert(ret >= 0);
	
		//listen不能注册EPOLLONESHOT事件，否则只能处理一个客户连接
		epoll_util::addfd(epfd, serverSocket->fd, false);
		return serverSocket->fd;
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
			for(int i=0;i<number;i++)
				_acceptor.handleAccept(serverSocket);
		}
		SocketFactory::destroy(serverSocket);
	}
	
	void MainReactor::NewConnectCallback(SocketImpl*& clientSok)
	{
		_disPatchcb(clientSok);
	}
}