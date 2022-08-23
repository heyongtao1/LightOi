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
#include "LSocket/socketfactory.h"
#include "LThread/LThreadPoolManage.h"
#include "LJob/LJob.h"
#include "Logger/Logger.h"
#include "LSocket/UDP/udp.h"
#include "config.hpp"

using namespace socketfactory;

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
		~MainReactor(){}
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
		SubReactor():activeNumber(0),totalActiveNumber(0),\
		poolmanage(std::make_shared<LThreadPoolManage<HYT::LJob>>(MAX_THREAD_NUMBER)){}
	
		~SubReactor()
		{
			for(auto it=users.begin();it != users.end();)
			{
				delete it->second;
				it->second = nullptr;
				it = users.erase(it);
			}
			users.clear();
			std::cout << "~SubReactor()" << endl;
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
					LOGERROR(NULL);
					continue;
				}
				for(int i=0;i<number;i++)
				{
					LOGINFO(NULL);
					int sockfd = events[i].data.fd;
					//onReadable
					if(events[i].events & EPOLLIN)
					{
						on_read(sockfd);
					}
					//onWriteable
					else if(events[i].events & EPOLLOUT)
					{
						on_write(sockfd);
					}
					// other Event
					else {}
				}
			}
		}
	private:
		void on_read(int fd)
		{
			if(users[fd]->read())
			{
				poolmanage->addTask(users[fd]);
				activeNumber++;
				totalActiveNumber++;
			}
			else
			{
				// close client fd
				LOGINFO(NULL);
				users[fd]->close_conn();
				{
					T* t = users[fd];
					delete t;
					t = nullptr;
				}
				users.erase(fd);
				activeNumber--;
			}
		}
		void on_write(int fd)
		{
			if(!users[fd]->write())
			{
				LOGINFO(NULL);
				users[fd]->close_conn();
				{
					T* t = users[fd];
					delete t;
					t = nullptr;
				}
				users.erase(fd);
			}
			activeNumber--;
		}
	public:
		// 加入新连接到监听事件中，接口
		void joinEPollEvent(SocketImpl*& clientSok)
		{ 
			LOGINFO(NULL);
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
		std::shared_ptr<LThreadPoolManage<HYT::LJob>> poolmanage;
	};
}
#endif
