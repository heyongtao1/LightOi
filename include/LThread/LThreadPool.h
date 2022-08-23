#ifndef _LTHREADPOOL_H
#define _LTHREADPOOL_H
#include <algorithm>
#include <vector>
#include <queue>
#include "LWorkerThread.h"
#include "Logger/Logger.h"
#include "config.hpp"
#include "LJob/LJob.h"
#include <mutex>
#include <condition_variable>
using namespace HYT;

template <typename T>
class LThreadPool{
public:
	LThreadPool(int initThreadNum = 10)
	{
		m_maxThreadNum = MAX_WTHREAD_NUMBER;
		m_minThreadNum = MIN_WTHREAD_NUMBER;
		if(initThreadNum > m_maxThreadNum || initThreadNum < m_minThreadNum)
			initThreadNum = m_minThreadNum;
		m_initThreadNum  = initThreadNum;
		m_allThread.clear();
		this->m_workId = 0;
		is_exit = false;
		increaseThread(m_initThreadNum);
	}
	~LThreadPool()
	{
		is_exit = true;
		m_workCond.notify_all();
		sleep(1);
		for (int i=0; i<m_allThread.size(); ++i)
		{
			delete m_allThread[i];
			m_allThread[i] = nullptr;
		}
		
#ifdef  DEBUG_COUT
		std::cout << "m_allThread.size() = " << m_allThread.size()<< std::endl;
#endif
		m_allThread.clear();
		vector<LWorkerThread<T>*>(m_allThread).swap(m_allThread);
		m_workque = queue<T*>();
	}
public:	
	template <typename U> friend class LWorkerThread;
public:
	static void* worker(void *arg)
	{
		LThreadPool* m_threadPool = (LThreadPool*)arg;
		while(!m_threadPool->is_exit)
		{
			std::unique_lock<std::mutex> guard(m_threadPool->m_workLock);
			while( !m_threadPool->is_exit && m_threadPool->m_workque.empty())
			{
				m_threadPool->m_workCond.wait(guard);
			}
			if(m_threadPool->is_exit) 
			{
				guard.unlock();
				std::cout << "thread exit" << std::endl;
				break;
			}
			if(!m_threadPool->m_workque.empty())
			{
				//取任务
				auto job = m_threadPool->m_workque.front();
				m_threadPool->m_workque.pop();
				guard.unlock();
				job->task();
			}
			else guard.unlock();
		}
		LightOi::LDebug::ldebug("WorkerThread Terminate");
		return NULL;
	}
	//增加多个工作线程到空闲队列
	void increaseThread(int num)
	{
		{
			for(int i=0;i<num;i++)
			{
				LWorkerThread<T>* workthread = new LWorkerThread<T>();
				pthread_create(&workthread->m_pthreadId,NULL,worker,this);
				workthread->setDetach(true);
				pthread_detach(workthread->m_pthreadId);

				m_allThread.push_back(workthread);
			}
			m_initThreadNum += num;
		}
	}
	void  decreaseThread(int num)
	{
		{
			std::lock_guard<std::mutex> guard(m_maxThreadNumLock);
			for(int i=0;i<num;i++)
			{

			}
		}
	}

public:
    void Run(T* job)
	{
		if(job == nullptr) return;
		{
			{
				std::lock_guard<std::mutex> guard(m_workLock);
				std::cout << m_workque.size() << std::endl;
				if(MAX_WORK_NUMBER <= m_workque.size() && m_initThreadNum < m_maxThreadNum)
				{
					std::cout << "increase thread" << std::endl;
					increaseThread(m_maxThreadNum - m_initThreadNum);
				}
				/*
				else if(MAX_WORK_NUMBER <= m_workque.size() && m_initThreadNum >= m_maxThreadNum)
				{
					//拒绝策略，丢弃任务但不抛出异常，输出日志
					std::cout << "refuse work" << std::endl;
					LOGINFO(NULL);
					return ;
				}
				*/
				m_workque.push(job);
			}
			m_workCond.notify_all();
		}
	}
private:
	//最大线程数
	int m_maxThreadNum;
	//最小线程数
	int m_minThreadNum;
	//初始化线程数
	int m_initThreadNum;
public:
	//所有线程的队列
	std::vector<LWorkerThread<T>*> m_allThread;
public:
	//保护最大线程数的互斥锁
	std::mutex m_maxThreadNumLock;
	//任务队列的互斥锁
	std::mutex m_workLock;
	
	//设置最大线程数的条件变量
	std::condition_variable m_maxThreadNumCond;
	//任务队列的条件变量
	std::condition_variable m_workCond;
	
	//任务队列
	std::queue<T*> m_workque;

	//任务编号
	unsigned int m_workId;

	bool is_exit;
};


#endif