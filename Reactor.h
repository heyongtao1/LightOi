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
#include "Acceptor.h"
#include "http_conn.h"
#include "threadpool.h"
#define MAX_CONN_EVENT_NUMBER 1000

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
		
		// loop epoll_wait
		virtual void loop() = 0;
	public:	
		int epfd;
		
		bool quit;
		
		epoll_event events[MAX_CONN_EVENT_NUMBER];
	};

	class MainReactor : public Reactor{
	typedef std::function<void(int)> disPatchCallback;
	public:
		MainReactor(const char *address, uint16_t port) : _address(address), _port(port)
		{
			// 设置已接收新连接的回调函数
			_acceptor.setNewConnectCallbackFun(std::bind(&MainReactor::NewConnectCallback,this,std::placeholders::_1));
		}
		
		~MainReactor()
		{
			stopLoop();
			EPollstop();
		}
	public:
		void NewConnectCallback(int newconfd);
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
	};
	template<typename T>
	class SubReactor : public Reactor{
	public:
		SubReactor(){}
	
		~SubReactor(){}

		// loop epoll_wait
		void loop() override
		{
			EPollstart();
			while(!quit)
			{
				// 1s内没有监听到活动事件，则返回处理是否有新连接加入
				int number = epoll_wait(epfd, events, MAX_CONN_EVENT_NUMBER, 1000);
				for(int i=0;i<number;i++)
				{
					int sockfd = events[i].data.fd;
					//onReadable
					if(events[i].events & EPOLLIN)
					{
						if(users[sockfd].read())
						{
							// join workthreadPoll 
							workthreadPoll.addjob(&users[sockfd]);
						}
						else
						{
							// close client fd
							users[sockfd].close_conn();
							users.erase(sockfd);
						}
					}
					//onWriteable
					else if(events[i].events & EPOLLOUT)
					{
						if(!users[sockfd].write())
						{
							users[sockfd].close_conn();
						}
					}
					// other Event
				}
				// handle new connect fd
				addEPollEvent();
			}
		}
		// 必须等到epoll_wait返回才能处理新客户连接
		void addEPollEvent()
		{
			while(!addfdlist.empty())
			{
				int fd = addfdlist.front();
				addfdlist.pop_front();
				T t;
				users.emplace(fd,std::move(t));
				users[fd].init(epfd,fd);
			}
		}
	public:
		// 加入新连接到监听事件链表中，接口
		void joinEPollEventList(int newClientfd)
		{ addfdlist.push_back(newClientfd); }
	private:
		std::list<int> addfdlist;
		std::unordered_map<int,T> users;
		// 计算线程池
		threadpool<http_conn> workthreadPoll;
	};
}
#endif