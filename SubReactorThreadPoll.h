/**
 * @file SubReactorThreadPool.h
 * @author HYT
 * @date 2021.11.17
 * @brief SubReactor Thread Pool
 */
#ifndef _SUBREACTORTHREADPOLL_H
#define _SUBREACTORTHREADPOLL_H
#include "SubReactorThread.h"
#define THREAD_MAX_NUM 8
namespace LightOi
{
	template <typename T>
	class SubReactorThreadPool{
	public:
		SubReactorThreadPool()
		{
			last = 0;
		}

	public:
		//round robin算法（类似负载均衡），将新连接合理分配到子Reactor进行监听
		void roundRobin(int newConnfd)
		{
			/* 算法 暂时 省略*/
			_subReactorThread[(++last)%THREAD_MAX_NUM].addNewConnectEvent(newConnfd);
		}
		
		void addNewConnectToSubReactor(int newConnfd)
		{	
			roundRobin(newConnfd);
		}
	private:
		SubReactorThread<T> _subReactorThread[THREAD_MAX_NUM];
		
		int last;
	};
}
#endif