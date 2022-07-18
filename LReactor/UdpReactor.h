#ifndef _UDP_REACTOR_H
#define _UDP_REACTOR_H
#include "Reactor.h"
#include "../LSocket/UDP/udp.h"
#include "../LSocket/socketfactory.h"
#include "../Lthread/LThreadPoolManage.h"
#include "../User/blog.h"
#include "../User/UDP_user.h"
#include "../Llib/Logger.h"
#define UDP_THREAD_NUM 1
namespace LightOi
{
    template <typename T>
  	class UdpReactor : public Reactor{
	public:
		UdpReactor()
		{
			activeNumber = totalActiveNumber = 0;
			poolmanage = new LThreadPoolManage<T>(UDP_THREAD_NUM);
		}
	
		~UdpReactor()
		{
			if(poolmanage != nullptr)
			{
				delete poolmanage;
				poolmanage = nullptr;
			}
            if(udp_main != nullptr)
            {
                delete udp_main;
                udp_main = nullptr;
            }
		}
		// loop epoll_wait
		void loop() override
		{
			EPollstart();
            udp_main = new protocol::udp_server(epfd);
            udp_fd = udp_main->create_udp_server(5555);
            cout << "udpReactor start" << endl;
			while(!quit)
			{
				// 若没有监听到活动事件，则阻塞
				int number = epoll_wait(epfd, events, MAX_CONN_EVENT_NUMBER, -1);
				if(number == -1) LogError(NULL);

				for(int i=0;i<number;i++)
				{
					LogInfo(NULL);
                    cout << "udp msg" << endl;
					int sockfd = events[i].data.fd;
					//onReadable
					if(events[i].events & EPOLLIN)
					{
                        // join workthreadPoll 
                        T udp_user(sockfd);
                        poolmanage->addTask(&udp_user);
                        activeNumber++;
                        totalActiveNumber++;
					}
				}
			}
		}
	private:
		protocol::udp_server* udp_main;
		int udp_fd;
        // 活动请求数目
        int activeNumber;
        int totalActiveNumber;
        // 计算线程池
        LThreadPoolManage<T>* poolmanage;
	};  
}
#endif