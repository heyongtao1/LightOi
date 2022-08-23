#ifndef _UDP_REACTOR_H
#define _UDP_REACTOR_H
#include "Reactor.h"
#include "LSocket/UDP/udp.h"
#include "LSocket/socketfactory.h"
#include "LThread/LThreadPoolManage.h"
#include "LJob/LJob.h"
#include "LJob/UDP_user.h"
#include "Logger/Logger.h"
#include "common_component/debug/LDebug.h"
#include "config.hpp"
#include <memory>
namespace LightOi
{
    template <typename T>
  	class UdpReactor : public Reactor{
	public:
		UdpReactor() : poolmanage{std::make_shared<LThreadPoolManage<T>>(UDP_THREAD_NUM)}
		{
			activeNumber = totalActiveNumber = 0;
			//poolmanage = new LThreadPoolManage<T>(UDP_THREAD_NUM);
		}
	
		~UdpReactor()
		{

		}
		// loop epoll_wait
		void loop() override
		{
			EPollstart();
			udp_main = std::make_shared<protocol::udp_server>(epfd);
            //udp_main = new protocol::udp_server(epfd);
            udp_fd = udp_main->create_udp_server(5555);
			LDebug::ldebug("udpReactor start");
			while(!quit)
			{
				// 若没有监听到活动事件，则阻塞
				int number = epoll_wait(epfd, events, MAX_CONN_EVENT_NUMBER, -1);
				if(number == -1) LOGERROR(NULL);

				for(int i=0;i<number;i++)
				{
					LOGINFO(NULL);
					LDebug::ldebug("udp msg");
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
		std::shared_ptr<protocol::udp_server> udp_main;
		int udp_fd;
        // 活动请求数目
        int activeNumber;
        int totalActiveNumber;
        // 计算线程池
        std::shared_ptr<LThreadPoolManage<T>> poolmanage;
		//LThreadPoolManage<T>* poolmanage;
	};  
}
#endif