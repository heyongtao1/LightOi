#ifndef _LWORKERTHREAD_H
#define _LWORKERTHREAD_H
#include "LThread.h"
#include "../User/LJob.h"
#include "Llock.h"
#include <mutex>
#include <condition_variable>
#include "../common_component/debug/LDebug.h"
using namespace HYT;

//线程状态
typedef enum _ThreadState
{
    THREAD_RUNNING = 0x0, //运行
    THREAD_IDLE = 0x1,//空闲
    THREAD_EXIT = 0X2,//退出
}ThreadState;

template <typename T> class LThreadPool;

template <typename T>
class LWorkerThread : public LThread {
public:
	LWorkerThread()
	{
		pthread_create(&this->m_pthreadId,NULL,this->worker,this);
		this->setDetach(true);
		pthread_detach(this->m_pthreadId);
		this->m_Job = nullptr;
		this->m_threadPool = nullptr;
	}
	~LWorkerThread()
	{
		this->Terminate();
	}
private:
	static void* worker(void *arg)
	{
		LWorkerThread* lwthread = (LWorkerThread*)arg;
		if(!lwthread->getStart());
		while(!lwthread->getTerminate())
		{
			std::unique_lock<std::mutex> guard(lwthread->m_varLocker);
			while(lwthread->m_Job == nullptr && !lwthread->getTerminate())
			{
				lwthread->m_isJobCond.wait(guard);
			}
			guard.unlock();
			if(lwthread->m_Job != nullptr)
			lwthread->Run();
		}
		LightOi::LDebug::ldebug("WorkerThread Terminate");
		return NULL;
	}
public:
	//接口
	void Run()
	{
		if(this->m_Job != nullptr)
		{
			this->m_workLocker.lock();
			
			this->m_Job->task();
			this->m_threadPool->moveBusyThreadToIdleQueue(this);
			//空闲线程过多，移除多余空闲线程
			if(m_threadPool->m_idleThread.size() > m_threadPool->getMaxIdleNum())
			{
				m_threadPool->deleteWorkThreadFromIdleQueue(m_threadPool->m_idleThread.size() - m_threadPool->getInitThreadNum());
			}
			this->m_Job = nullptr;
			this->m_workLocker.unlock();
		}
	}
	void Start()
	{
		this->setStart(true);
	}
	void Terminate()
	{
		//工作线程再处理任务结束才会解锁，这个时候再去退出线程，避免打断线程处理任务。
		this->m_workLocker.lock();
		
		this->setTerminate(true);
		this->m_Job = nullptr;
		this->m_isJobCond.notify_all();
		//不加延迟可能会导致出现pure virtual method called called terminate called without a active exception
		//加延迟是为了等待线程执行完任务后，不再调用该线程对象其他成员函数，再最后销毁该线程
		usleep(1000);
		
		this->m_workLocker.unlock();
	}
private:
	//执行的任务
	T* m_Job;
	//所在的线程池对象
	LThreadPool<T>* m_threadPool;
	//任务编号
	unsigned int m_workId;
private:
	//任务的条件变量，等待执行任务和唤醒执行任务
	std::condition_variable m_isJobCond;
	//唤醒任务的锁
	std::mutex m_isnotify;
	//对于设置任务、设置线程池对象的锁
	std::mutex m_varLocker;
public:
	//任务执行的锁，防止多个线程执行任务
	LThreadlocker m_workLocker;
public:
	void setLJob(T* job)
	{
		if(job == nullptr) return ;
		{
			std::lock_guard<std::mutex> guard(m_varLocker);
			this->m_Job = job;
		}
		m_isJobCond.notify_all();
	}
	void setThreadPool(LThreadPool<T>* threadpool)
	{
		if(threadpool == nullptr) return ;
		{
			std::lock_guard<std::mutex> guard(m_varLocker);
			this->m_threadPool = threadpool;
		}
	}
	void setWorkId(unsigned int workId)
	{
		this->m_workId = workId;
	}
};

#endif