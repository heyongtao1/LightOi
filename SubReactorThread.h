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
#include "Reactor.h"
/*
	模板类并非真正的类，模板类的实现和定义必须在同一头文件中
*/
namespace LightOi
{
	template <typename T>
	class SubReactorThread{
	public:
		SubReactorThread()
		{
			if(pthread_create(&_thread, NULL, work, this)!=0)
			{
				throw std::exception();
			}
			//将线程分离,任务执行完直接销毁
			if(pthread_detach(_thread))
			{
				throw std::exception();
			}
			std::cout << "SubReactorThread start" <<std::endl;
		}

	public:
		static void* work(void* arg)
		{
			SubReactorThread *pool = (SubReactorThread*)arg;
			// 子Reactor不断循环监听
			pool->run();
			return pool;
		}
		void run()
		{
			_subReactor.loop();
		}
		// 接口 ：新客户连接加入到子Reactor进行监听活动事件
		void addNewConnectEvent(int newConnfd)
		{ _subReactor.joinEPollEventList(newConnfd); }
	private:
		pthread_t _thread;
		
		SubReactor<T> _subReactor;
	};

}
#endif