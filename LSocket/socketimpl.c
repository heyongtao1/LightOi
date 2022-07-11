/*
  2  * socketimpl.cpp
  3  *
  4  *  Created on: 2014-7-20
  5  *      Author: root
  6  */
#include <unistd.h>
#include "socketimpl.h"
#include <sys/types.h>
#include <sys/socket.h> 
#include <stdio.h>
 
 
SocketImpl::SocketImpl() {
     this->fd = -1;
}
 
int SocketImpl::ssocket(int domain, int type, int protocol) {
     this->fd = socket(domain, type, protocol);
     if (this->fd < 0)
         perror("SocketImpl::ssocket");
     return this->fd;
}
 
int SocketImpl::cconnect(int sockfd, const struct sockaddr_in *addr,
         socklen_t addrlen) {
     int ret = connect(sockfd, (struct sockaddr*)addr, addrlen);
     if (ret != 0)
         perror("SocketImpl::cconnect");
     return ret;
 
}

SocketImpl::~SocketImpl() {
 
}
 
int SocketImpl::read(void* buf, size_t len) {
     int ret = ::read(this->fd, buf, len);
     if (ret < 0)
         perror("SocketImpl::read");
     return ret;
}
 
int SocketImpl::write(void* buf, size_t len) {
     int ret = ::write(this->fd, buf, len);
     if (ret < 0)
         perror("SocketImpl::write");
     return ret;
}
 
int SocketImpl::close() {
     if (this->fd > 0) {
         int ret = ::close(this->fd);
         if (ret != 0)
         {
             perror("SocketImpl::close");
             return ret;
         }else
             this->fd = -1;
     }
     return 0;
}
 
	ServerSocketImpl::ServerSocketImpl() {
    this->fd = 0;
}
 
 ServerSocketImpl::~ServerSocketImpl() {
}
 
int ServerSocketImpl::ssocket(int domain, int type, int protocol) {
     this->fd = socket(domain, type, protocol);
     if (this->fd < 0)
         perror("ServerSocketImpl::ssocket");
     return this->fd;
}
 
int ServerSocketImpl::bbind(int sockfd, const struct sockaddr_in *addr,
         socklen_t addrlen) {
     int ret = bind(this->fd, (struct sockaddr*)addr, addrlen);
     if (ret < 0)
         perror("ServerSocketImpl::bbind");
     return ret;
}

SocketImpl* ServerSocketImpl::accept() {
     SocketImpl* nsocket = new SocketImpl();
     socklen_t addresslength = sizeof(nsocket->address);
     int nfd =  ::accept(this->fd, (struct sockaddr*)&nsocket->address, &addresslength);
     if (nfd == -1) {
         delete nsocket;
         perror("ServerSocketImpl::accept");
         return NULL;
     }
     nsocket->fd = nfd;
     return nsocket;
}
 
int ServerSocketImpl::listen(int backlog) {
     int ret = ::listen(this->fd, backlog);
     if (ret < 0)
         perror("ServerSocketImpl::listen");
     return ret;
}
 
int ServerSocketImpl::close() {
     if(this->fd >0 )
     {
         int ret=::close(this->fd);
         if(ret!=0 )
         {
             perror("ServerSocketImpl::close");
             return ret;
         }else
             this->fd =-1;
     }
     return 0;
}