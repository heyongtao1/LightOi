/**
 * @file SubReactorThreadPool.h
 * @author HYT
 * @date 2021.11.17
 * @brief SubReactor Thread Pool
 */
#ifndef _SUBREACTORTHREADPOLL_H
#define _SUBREACTORTHREADPOLL_H
#include <iostream>
#include "SubReactorThread.h"
#include "../Llib/Logger.h"
#include "../LSocket/socketimpl.h"
#define THREAD_MAX_NUM 1
using namespace socketfactory;
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
		void roundRobin(SocketImpl*& clientSok)
		{
			int start = (last + 1) % THREAD_MAX_NUM;
			int min_load_idx = start;
			int min_load_reqs = _subReactorThread[start].getActiveNumber();
			
			for(int i=0;i < THREAD_MAX_NUM;i++)
			{
				int idx = (start + i ) % THREAD_MAX_NUM;
				int load = _subReactorThread[idx].getActiveNumber();
				if( load < min_load_reqs)
				{
					min_load_reqs = load;
					min_load_idx = idx;
					if(min_load_reqs == 0)
						break;
				}
			}
			
			last = min_load_idx;		
			LogInfo(NULL);
			std::cout << "activtNumber = " << _subReactorThread[min_load_idx].getActiveNumber() << std::endl;
			_subReactorThread[min_load_idx].addNewConnectEvent(clientSok);
			//_subReactorThread[(++last)%THREAD_MAX_NUM].addNewConnectEvent(newConnfd);
		}
		
		void addNewConnectToSubReactor(SocketImpl*& clientSok)
		{	
			roundRobin(clientSok);
		}
		
		void stopTotalSubReactor()
		{
			for(int i=0;i<THREAD_MAX_NUM;i++)
				_subReactorThread[i].stopWork();
		}
		
		void printTotalActiveNumber()
		{
			for(int i=0;i<THREAD_MAX_NUM;i++)
				std::cout << " i activeNumber = "  << std::endl;//<< _subReactorThread[i].getTotalActiveNumber()
		}
	private:
		SubReactorThread<T> _subReactorThread[THREAD_MAX_NUM];
		
		int last;
	};
}
#endif