#include "udp.h"

namespace protocol
{
    int udp_server::create_udp_server()
    {
        int sockfd = create_udp_socket();
        
        epoll_util::addfd(epfd,sockfd,true);
        int port = rand_port();
        fd_port[sockfd] = port;
        int groupId = rand_groupId();
        fd_groupId[sockfd] = groupId;
        groupId_fd[groupId] = sockfd;
        bind_udp(sockfd,port);
        return sockfd;
    }

    int udp_socket::create_udp_socket()
    {
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        return fd;
    }
    void udp_socket::bind_udp(int sockfd,int port)
    {
         struct sockaddr_in serveraddr;
         memset(&serveraddr, 0, sizeof(struct sockaddr_in));
 
         serveraddr.sin_family = AF_INET;
         serveraddr.sin_port = htons(port);
         serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
 
         bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    }

    udp_socket::~udp_socket()
    {
        close(fd);
    } 
    
    void udp_server::close_udp(int sockfd)
    {
        {
            ports.erase(fd_port[sockfd]);
            groupIds.erase(fd_groupId[sockfd]);
            fd_port.erase(sockfd);
            fd_groupId.erase(sockfd);
            groupId_fd.erase(fd_groupId[sockfd]);
        }
        epoll_util::removefd(epfd,sockfd);
        close(sockfd);
    }

    int udp_server::rand_port()
    {
        int port;
        while(true)
        {
            srand((unsigned)time(NULL));
            port = (rand()%30000)+5000;
            if(ports.find(port) == ports.end())
            {
                ports.insert(port);
                break;
            }
        }
        return port;
    }
    unsigned long udp_server::rand_groupId()
    {
        unsigned long groupId;
        while(true)
        {
            srand((unsigned)time(NULL));
            groupId = (rand()%999999)+100000;
            if(groupIds.find(groupId) == groupIds.end())
            {
                groupIds.insert(groupId);
                break;
            }
        }
        return groupId;
    }
    udp_server::~udp_server()
    {
        for(auto it=ports.begin();it != ports.end(); it++)
            close_udp(*it);
            
        ports.clear();
        groupIds.clear();
        fd_port.clear();
        fd_groupId.clear();
        groupId_fd.clear();
    }
}