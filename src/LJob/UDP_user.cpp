#include "UDP_user.h"
#include <iostream>
bool UDP_user::recv_data(int sockfd)
{
    std::cout << "recv data" << std::endl;
    addrlen = sizeof(clientaddr);
    //接收包头
    int recvlen = 1024;
    //int result = recvfrom(sockfd, (char*)&recvlen, sizeof(recvlen), 0, (sockaddr*)&clientaddr, &addrlen);
    char sockbuf[recvlen+1] = {'\0'};
    int result  = recvfrom(sockfd, sockbuf, sizeof(sockbuf), 0, (sockaddr*)&clientaddr, &addrlen);
    printf("sockf msg = %s\n",sockbuf);
    //处理逻辑
    {

    }
    //响应
    if(!send_data(sockfd,sockbuf,strlen(sockbuf))) return false;
    return true;
}
bool UDP_user::send_data(int sockfd,void *data,int len)
{
    if(sendto(sockfd, (char*)data, len, 0, (sockaddr*)&clientaddr, addrlen) == 0)
        return false;
    return true;
}

void UDP_user::task()
{
    recv_data(sockfd);
}