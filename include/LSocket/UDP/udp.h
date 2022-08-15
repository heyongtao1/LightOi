#ifndef _UDP_H
#define _UDP_H
#include <stdio.h>                                                                                                                                                                                             
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <vector>
#include <map>
#include <set>
#include "util/util.h"
namespace protocol{

#define SERV_PORT 8888

class udp_socket{
public:
    udp_socket(){}
    ~udp_socket(){};
public:
    int  create_udp_socket();
    void bind_udp(int sockfd,int port);
private:
    int fd;
};

class udp_server : public udp_socket{
public:
    udp_server(int fd): epfd(fd){}
    ~udp_server();
public:
    //interface
    int  create_udp_server(int port);
    void close_udp(int sockfd);
public:         
    int  rand_port();
    unsigned long rand_groupId();
public:
    std::map<int,int> fd_port;
    std::set<int> ports;
    std::map<int,unsigned long> fd_groupId;
    std::map<unsigned long,int> groupId_fd;
    std::set<unsigned long> groupIds;
    int epfd;
};

}

#endif