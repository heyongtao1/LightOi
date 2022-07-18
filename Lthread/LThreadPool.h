#ifndef _LTHREADPOOL_H
#define _LTHREADPOOL_H
#include <algorithm>
#include <vector>
#include "LWorkerThread.h"
#include "Llock.h"
#include "../User/blog.h"
#include <mutex>
#include <condition_variable>
using namespace HYT;

template <typename T>
class LThreadPool{
public:
	LThreadPool(int initThreadNum = 50)
	{
		m_realIdleNum = m_initThreadNum = initThreadNum;
		m_maxThreadNum = 100;
		m_maxIdleNum = m_maxThreadNum - 10;
		m_minIdleNum = (m_initThreadNum - 10) > 0 ? 10 : 5;

		m_allThread.clear();
		m_idleThread.clear();
		m_busyThread.clear();

		this->m_workId = 0;

		createWorkThreadToIdleQueue(m_initThreadNum);
	}
	~LThreadPool()
	{
		TerminateAll();
		m_allThread.clear();
		m_idleThread.clear();
		m_busyThread.clear();
	}
public:	
	template <typename U> friend class LWorkerThread;
public:
	//将空闲线程加入到忙碌队列
	void moveIdleThreadToBusyQueue(LWorkerThread<T>* idleThread)
	{
		if(idleThread == nullptr) return ;

		{
			std::lock_guard<std::mutex> gurad(m_idlequeLock);
			auto pos = std::find(m_idleThread.begin(),m_idleThread.end(),idleThread);
			if(pos != m_idleThread.end())
			m_idleThread.erase(pos);
		}
		{
			std::lock_guard<std::mutex> gurad(m_busyqueLock);
			m_busyThread.push_back(idleThread);
		}
	}
	//将忙碌线程加入到空闲队列
	void moveBusyThreadToIdleQueue(LWorkerThread<T>* busyThread)
	{
		if(busyThread == nullptr) return ;

		{
			std::lock_guard<std::mutex> gurad(m_busyqueLock);
			auto pos = std::find(m_busyThread.begin(),m_busyThread.end(),busyThread);
			if(pos != m_busyThread.end())
			m_busyThread.erase(pos);
		}

		{
			std::lock_guard<std::mutex> gurad(m_idlequeLock);
			m_idleThread.push_back(busyThread);
		}
		m_idlequeCond.notify_all();
		m_maxThreadNumCond.notify_all();
	}
	//获取空闲线程
	LWorkerThread<T>* getIdleThread()
	{
		std::unique_lock<std::mutex> gurad(m_idlequeLock);
		while(m_idleThread.size() == 0)
		{
			m_idlequeCond.wait(gurad);
		}

		if(m_idleThread.size() > 0)
		{
			LWorkerThread<T>* idlethread = m_idleThread.front();
			gurad.unlock();
			return idlethread;
		}

		//获取失败
		gurad.unlock();
		return nullptr;
	}
	//增加一个工作线程到空闲队列
	void appendWorkThreadToIdleQueue(LWorkerThread<T>* workthread)
	{
		if(workthread == nullptr) return ;
		{
			std::lock_guard<std::mutex> gurad(m_idlequeLock);
			m_allThread.push_back(workthread);
			m_idleThread.push_back(workthread);
		}
	}
	//增加多个工作线程到空闲队列
	void createWorkThreadToIdleQueue(int num)
	{
		std::cout << "LThreadPool::createWorkThreadToIdleQueue num = " << num << std::endl;
		{
			std::lock_guard<std::mutex> gurad(m_idlequeLock);
			for(int i=0;i<num;i++)
			{
				LWorkerThread<T>* workthread = new LWorkerThread<T>();
				workthread->setThreadPool(this);

				m_allThread.push_back(workthread);
				m_idleThread.push_back(workthread);

				workthread->Start();
			}
		}

		m_idlequeCond.notify_all();
		m_maxThreadNumCond.notify_all();
	}
	//从空闲队列删除多个空闲线程
	void deleteWorkThreadFromIdleQueue(int num)
	{
		std::lock_guard<std::mutex> gurad(m_idlequeLock);
		for(int i=0;i<num;i++)
		{
			LWorkerThread<T>* deltThread;
			if(m_idleThread.size() > 0)
			{
				deltThread = m_idleThread.front();
			}
			else
			break;
			auto pos = std::find(m_idleThread.begin(),m_idleThread.end(),deltThread);
			if(pos != m_idleThread.end())
				m_idleThread.erase(pos);
			//m_realIdleNum--;
		}

		std::cout << "LThreadPool::deleteWorkThreadFromIdleQueue num = " << num << std::endl;
	}

public:
    void TerminateAll()
	{
		for (int i=0; i<m_allThread.size(); ++i)
		{
			delete m_allThread[i];
			m_allThread[i] = nullptr;
		}
		std::cout << "m_allThread.size() = " << m_allThread.size()<< std::endl;
		std::cout << "realuseThread = " << m_idleThread.size() + m_busyThread.size()<< std::endl;
	}
    void Run(T* job)
	{
		if(job == nullptr) return;
		{
			std::unique_lock<std::mutex> guard(m_maxThreadNumLock);
			while(m_maxThreadNum <= m_busyThread.size())
			{
				m_maxThreadNumCond.wait(guard);
			}		
		}

		int alluseThreadNum = m_busyThread.size() + m_idleThread.size();
	
		//负载过重，空闲线程少，需要创建新的线程, 使其空闲线程数目达到m_InitThreadNum
		if(m_maxThreadNum > alluseThreadNum&& m_idleThread.size() < m_minIdleNum)
		{
			//注意：判断创建后线程总数是否大于m_maxThreadNum
			//小于或等于m_maxThreadNum，则创建m_initThreadNum - m_idleThread.size()个线程，
			//使得空闲线程数量达到m_initThreadNum
			if(m_maxThreadNum > (alluseThreadNum + m_initThreadNum - m_idleThread.size()))
			{
				createWorkThreadToIdleQueue(m_initThreadNum - m_idleThread.size());
			}
			else
			{
				createWorkThreadToIdleQueue(m_maxThreadNum - alluseThreadNum);
			}
		}
		//获取空闲线程去执行任务
		LWorkerThread<T>* workThread = getIdleThread();
		if(workThread != nullptr)
		{
			workThread->m_workLocker.lock();
			
			moveIdleThreadToBusyQueue(workThread);
			workThread->setThreadPool(this);
			workThread->setLJob(job);
			workThread->setWorkId(m_workId++);
			workThread->m_workLocker.unlock();
		}
	}
private:
	//最大线程数
	int m_maxThreadNum;
	//最大空闲线程数
	int m_maxIdleNum;
	//最小空闲线程数
	int m_minIdleNum;
	//实际空闲线程数
	int m_realIdleNum; // 介于m_minIdleNum 和 m_maxIdleNum之间
	//初始化线程数
	int m_initThreadNum;
public:
	void setMaxThreadNum(int maxThreadNum){ m_maxThreadNum = maxThreadNum;}
	int  getMaxThreadNum() { return m_maxThreadNum; }
	void setMaxIdleNum(int maxIdleNum){ m_maxIdleNum = maxIdleNum;}
	int  getMaxIdleNum() { return m_maxIdleNum; }
	void setMinIdleNum(int minIdleNum){ m_minIdleNum = minIdleNum;}
	int  getMinIdleNum() { return m_minIdleNum; }
	void setRealIdleNum(int realIdleNum){ m_realIdleNum = realIdleNum;}
	int  getRealIdleNum() { return m_realIdleNum; }
	void setInitThreadNum(int initThreadNum){ m_initThreadNum = initThreadNum;}
	int  getInitThreadNum() { return m_initThreadNum; }
public:
	//所有线程的队列
	std::vector<LWorkerThread<T>*> m_allThread;
	//空闲线程的队列
	std::vector<LWorkerThread<T>*> m_idleThread;
	//忙碌线程的队列
	std::vector<LWorkerThread<T>*> m_busyThread;
private:
	//保护空闲线程队列的互斥锁
	std::mutex m_idlequeLock;
	//保护忙碌线程队列的互斥锁
	std::mutex m_busyqueLock;
	//保护实际空闲线程的互斥锁
	std::mutex m_realIdleNumLock;//已经废弃
	//保护最大线程数的互斥锁
	std::mutex m_maxThreadNumLock;
	
	//条件变量
	std::condition_variable m_busyqueCond;
	std::condition_variable m_idlequeCond;
	//设置最大线程数的条件变量
	std::condition_variable m_maxThreadNumCond;
	
	//任务编号
	unsigned int m_workId;
};


#endif