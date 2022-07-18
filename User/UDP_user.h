#ifndef _UDP_USER_H
#define _UDP_USER_H
#include "../LSocket/UDP/udp.h"
class UDP_user{
public:
	UDP_user(int fd):sockfd(fd){}
    ~UDP_user(){}
public:
	bool recv_data(int sockfd);
	bool send_data(int sockfd,void *data,int len);
    void task();
private:
    int sockfd;
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
};

#endif