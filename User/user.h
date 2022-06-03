#ifndef _USER_H
#define _USER_H
#include <iostream>
#include "../Llib/Logger.h"
#include "../util/util.h"
#include "../LSocket/socketimpl.h"
using namespace socketfactory;
class User {
public:
	User():clientSok(nullptr){}
	~User(){}
public:
	void init(int _epollfd,SocketImpl*& _clientSok)
	{
		LogInfo(NULL);
		epollfd = _epollfd;
		clientSok = _clientSok;
		epoll_util::addfd(epollfd,clientSok->fd,true);
	}
	void writable()
	{
		epoll_util::modfd(epollfd,clientSok->fd,EPOLLOUT);
	}
	
	void readable()
	{
		epoll_util::modfd(epollfd,clientSok->fd,EPOLLIN);
	}
	
	void close_conn()
	{
		if(clientSok->fd != -1)
		{
			LogInfo(NULL);
			cout << "客户端fd = "<< clientSok->fd  << "下线"<<endl;
			epoll_util::removefd(epollfd,clientSok->fd);
			SocketFactory::destroy(clientSok);
		}
	}
	
	void task(){}
public:
	int epollfd;
	SocketImpl* clientSok;
};

#endif