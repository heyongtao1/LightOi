/*
 * socketfactory.cpp
 *
 *  Created on: 2014-7-20
 *      Author: root
 */

#include "socketfactory.h"
#include "socketimpl.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>


ISocket* SocketFactory::createSocket(const char* tIP, int tPort)
{
    SocketImpl*  nsocket=new SocketImpl();
    memset(&nsocket->address, 0, sizeof(sockaddr));
    struct sockaddr_in* padd=(sockaddr_in*)(&nsocket->address);
    padd->sin_family=AF_INET;
    padd->sin_port=htons(tPort);
    if( inet_pton(AF_INET, tIP, &padd->sin_addr) <= 0){
        delete nsocket;
        perror("SocketFactory::createSocket:inet_pton");
        return NULL;
    }
    int ret=nsocket->ssocket(AF_INET, SOCK_STREAM, 0);
    if(ret < 0 ){
        perror("SocketFactory::createSocket:ssocket");
        delete nsocket;
        return NULL;
    }
    ret=nsocket->cconnect(nsocket->fd, &nsocket->address, sizeof(sockaddr));
    if(ret < 0 ){
        perror("SocketFactory::createSocket:cconnect");
        nsocket->close();
        delete nsocket;
        return NULL;
    }
    return nsocket;
}

IServerSocket* SocketFactory::createServerSocket(int port)
{
    ServerSocketImpl *nssocket=new ServerSocketImpl();
    memset(&nssocket->address, 0, sizeof(sockaddr));
    nssocket->address.sin_family=AF_INET;
    nssocket->address.sin_addr.s_addr=htonl(INADDR_ANY);
    nssocket->address.sin_port=htons(port);
    int ret=nssocket->ssocket(AF_INET, SOCK_STREAM, 0);
    if(ret<0){
        perror("SocketFactory::createServerSocket:ssocket");
        delete nssocket;
        return NULL;
    }
    ret=nssocket->bbind(nssocket->fd, &nssocket->address, sizeof(sockaddr));
    if(ret<0){
        perror("SocketFactory::createServerSocket:bbind");
        nssocket->close();
        delete nssocket;
        return NULL;
    }
    return nssocket;
}

int SocketFactory::destroy(ISocket* psocket)
{
    SocketImpl* psockimpl=(SocketImpl*)psocket;
    psockimpl->close();
    delete psockimpl;
	psockimpl = nullptr;
    return 0;
}

int SocketFactory::destroy(IServerSocket* psocket)
{
    ServerSocketImpl* pssocket=(ServerSocketImpl*)psocket;
    pssocket->close();
    delete pssocket;
	pssocket = nullptr;
    return 0;
}