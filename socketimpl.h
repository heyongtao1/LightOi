/*
* socketimpl.h
*
*  Created on: 2014-7-20
*      Author: root
*/
 
#ifndef SOCKETIMPL_H_
#define SOCKETIMPL_H_
#include <sys/socket.h>
#include <netinet/in.h>
#include "socketfactory.h"
 
using namespace socketfactory;

/*
* ISocket 和 IServerSocket 的实现类。
*  SocketFactory工厂类创建这些实现类。
*
*/
 
class SocketImpl: public ISocket
{
public:
     int read(void* buf, size_t len);
     int write(void* buf, size_t len);
     int close();
 
     int ssocket(int domain, int type, int protocol);
     int cconnect(int sockfd, const struct sockaddr_in *addr, socklen_t addrlen);
     SocketImpl();
     virtual ~SocketImpl();
 
public:
     int fd;
     struct sockaddr_in  address;
};
 

class ServerSocketImpl: public IServerSocket
{
public:
     SocketImpl* accept();
     int listen(int backlog);
     int close();
 
 
     int ssocket(int domain, int type, int protocol);
     int bbind(int sockfd, const struct sockaddr_in *addr, socklen_t addrlen);
     ServerSocketImpl();
     virtual ~ServerSocketImpl();
 
public:
     int fd;
     struct sockaddr_in  address;
};


#endif /* SOCKETIMPL_H_ */