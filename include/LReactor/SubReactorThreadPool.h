/**
 * @file SubReactorThreadPool.h
 * @author HYT
 * @date 2021.11.17
 * @brief SubReactor Thread Pool
 */
#ifndef _SUBREACTORTHREADPOLL_H
#define _SUBREACTORTHREADPOLL_H
#include <iostream>
#include <memory>
#include <vector>
#include "Reactor.h"
#include "SubReactorThread.h"
#include "UdpReactor.h"
#include "Logger/Logger.h"
#include "LSocket/socketimpl.h"
#include "config.hpp"
using namespace socketfactory;
namespace LightOi
{
	template <typename T,typename U>
	class SubReactorThreadPool{
	public:
		SubReactorThreadPool():last(0){}
		~SubReactorThreadPool()
		{
			//_subReactorThread.clear();
			stopTotalSubReactor();
		}

	public:
		//round robin算法（类似负载均衡），将新连接合理分配到子Reactor进行监听
		void roundRobin(SocketImpl*& clientSok)
		{
			int start = (last + 1) % SUBREACTOR_THREAD_MAX_NUM;
			int min_load_idx = start;
			int min_load_reqs = _subReactorThread[start].getActiveNumber();
			
			for(int i=0;i < SUBREACTOR_THREAD_MAX_NUM;i++)
			{
				int idx = (start + i ) % SUBREACTOR_THREAD_MAX_NUM;
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
			LOGINFO(NULL);
#ifdef	DEBUG_COUT
			std::cout << "activtNumber = " << _subReactorThread[min_load_idx].getActiveNumber() << std::endl;
#endif
			_subReactorThread[min_load_idx].addNewConnectEvent(clientSok);
			//_subReactorThread[(++last)%THREAD_MAX_NUM].addNewConnectEvent(newConnfd);
		}
		
		void addNewConnectToSubReactor(SocketImpl*& clientSok)
		{	
			roundRobin(clientSok);
		}
		
		void stopTotalSubReactor()
		{
			for(int i=0;i<SUBREACTOR_THREAD_MAX_NUM;i++)
				_subReactorThread[i].stop();
			//_udpReactorThread.stopWork();
		}
		
		void printTotalActiveNumber()
		{
			for(int i=0;i<SUBREACTOR_THREAD_MAX_NUM;i++)
				std::cout << " i activeNumber = "<< _subReactorThread[i].getTotalActiveNumber()  << std::endl;
		}
	private:
		SubReactorThread<SubReactor<T>> _subReactorThread[SUBREACTOR_THREAD_MAX_NUM];
		//SubReactorThread<UdpReactor<U>> _udpReactorThread;
		int last;
	};
}
#endif