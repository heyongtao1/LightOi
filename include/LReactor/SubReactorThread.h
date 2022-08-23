/**
 * @file SubReactorThread.h
 * @author HYT
 * @date 2021.11.17
 * @brief SubReactor Thread
 */
#ifndef _SUBREACTORTHREAD_H
#define _SUBREACTORTHREAD_H
#include <pthread.h>
#include <exception>
#include <thread>
#include <memory>
#include "Reactor.h"
#include "LSocket/socketimpl.h"
#include "common_component/debug/LDebug.h"
using namespace socketfactory;

namespace LightOi
{
	template <typename T>
	class SubReactorThread{
	public:
		SubReactorThread()
		{
			if(pthread_create(&_thread, NULL, work, this)!=0){
				throw std::exception();
			}
			//将线程分离,任务执行完直接销毁
			if(pthread_detach(_thread)){
				throw std::exception();
			}
			LDebug::ldebug("SubReactorThread start");
		}
		~SubReactorThread()=default;
	public:
		static void* work(void* arg)
		{
			SubReactorThread *reactor = (SubReactorThread*)arg;
			reactor->run();
			return reactor;
		}
		void run()
		{
			subreactor.loop();
		}
		
		void stop()
		{
			subreactor.stop();
		}
		
		// 接口 ：新客户连接加入到子Reactor进行监听活动事件
		void addNewConnectEvent(SocketImpl*& clientSok)
		{ subreactor.joinEPollEvent(clientSok); }
		
		int getActiveNumber() { return subreactor.getActiveNumber(); }
		
		int getTotalActiveNumber() { return subreactor.getTotalActiveNumber(); }
	private:
		pthread_t _thread;
		T subreactor;
	};

}
#endif