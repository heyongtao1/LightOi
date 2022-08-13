/**
 * @file Reactor.h
 * @author HYT
 * @date 2021.11.17
 * @brief MainReactor and SubReactor inherit Reactor
 * MainReactor : Only responsible monitor for new connection events
 * SubReactor  : Only responsible monitor for connected active events
 */
#ifndef _REACTOR_H
#define _REACTOR_H

#include <fcntl.h>
#include <assert.h>
#include <sys/epoll.h>
#include <list>
#include <unordered_map>
#include <memory>
#include "Acceptor.h"
#include "../LSocket/socketfactory.h"
#include "../Lthread/LThreadPoolManage.h"
#include "../User/LJob.h"
#include "../Llib/Logger.h"
#include "../LSocket/UDP/udp.h"
#include "../config.hpp"

using namespace socketfactory;
//全局任务队列
//static threadpool<http_conn> workthreadPoll;

namespace LightOi
{
	class Reactor{
	public:
		Reactor() : quit(false) {}
		
		~Reactor(){}
	public:
		void stopLoop();
		
		// epoll_create
		void EPollstart();
		
		void EPollstop();
		
		void stop();
		
		// loop epoll_wait
		virtual void loop() = 0;
	public:	
		int epfd;
		
		bool quit;
		
		epoll_event events[MAX_CONN_EVENT_NUMBER];
	};

	class MainReactor : public Reactor{
	typedef std::function<void(SocketImpl*&)> disPatchCallback;
	public:
		MainReactor(const char *address, uint16_t port) : _address(address), _port(port)
		{
			// 设置已接收新连接的回调函数
			_acceptor.setNewConnectCallbackFun(std::bind(&MainReactor::NewConnectCallback,this,std::placeholders::_1));
		}
		~MainReactor()
		{
		}
	public:
		void NewConnectCallback(SocketImpl*& clientSok);
		// return listenfd
		int listens();
		
		void setdisPatchCallbackFun( disPatchCallback cb) 
		{ _disPatchcb = cb; }
		
		// loop epoll_wait
		void loop() override;
		
	private:	
		Acceptor _acceptor;
		
		const char* _address;
		
		uint16_t _port;
		// 将新客户连接分配到子Reactor的回调函数
		disPatchCallback _disPatchcb;
		
		ServerSocketImpl* serverSocket;
	};
	template<typename T>
	class SubReactor : public Reactor{
	public:
		SubReactor()
		{
			activeNumber = totalActiveNumber = 0;
			poolmanage = std::make_shared<LThreadPoolManage<HYT::LJob>>(MAX_THREAD_NUMBER);
			//poolmanage = new LThreadPoolManage<HYT::LJob>(MAX_THREAD_NUMBER);
		}
	
		~SubReactor()
		{
			for(auto it=users.begin();it != users.end();it++)
			{
				delete it->second;
				it->second = nullptr;
				users.erase(it++);
			}
			users.clear();
		}

		// loop epoll_wait
		void loop() override
		{
			EPollstart();
			while(!quit)
			{
				// 若没有监听到活动事件，则阻塞
				int number = epoll_wait(epfd, events, MAX_CONN_EVENT_NUMBER, -1);
				if(number == -1)
				{
					LogError(NULL);
				}
				if(number > 0)
				for(int i=0;i<number;i++)
				{
					LogInfo(NULL);
					int sockfd = events[i].data.fd;
					//onReadable
					if(events[i].events & EPOLLIN)
					{
						if(users[sockfd]->read())
						{
							poolmanage->addTask(users[sockfd]);
							activeNumber++;
							totalActiveNumber++;
						}
						else
						{
							// close client fd
							LogInfo(NULL);
							users[sockfd]->close_conn();
							{
								T* t = users[sockfd];
								delete t;
								t = nullptr;
							}
							users.erase(sockfd);
							activeNumber--;
						}
					}
					//onWriteable
					else if(events[i].events & EPOLLOUT)
					{
						if(!users[sockfd]->write())
						{
							LogInfo(NULL);
							users[sockfd]->close_conn();
							{
								T* t = users[sockfd];
								delete t;
								t = nullptr;
							}
							users.erase(sockfd);
						}
						activeNumber--;
					}
					// other Event
				}
			}
		}
	public:
		// 加入新连接到监听事件中，接口
		void joinEPollEvent(SocketImpl*& clientSok)
		{ 
			LogInfo(NULL);
			activeNumber++;
			
			T* t = new T();
			users.emplace(clientSok->fd,t);
			users[clientSok->fd]->init(epfd,clientSok);	
		}
		
		int getActiveNumber() { return activeNumber; }
		
		int getTotalActiveNumber() { return totalActiveNumber; }
	private:
		std::unordered_map<int,T*> users;
		// 活动请求数目
		int activeNumber;
		int totalActiveNumber;
		// 计算线程池
		//threadpool<HYT::LJob> workthreadPoll;
		std::shared_ptr<LThreadPoolManage<HYT::LJob>> poolmanage;
		//LThreadPoolManage<HYT::LJob>* poolmanage;
	};
}
#endif
