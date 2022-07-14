#include "LWorkerThread.h"
#include "LThreadPool.h"
#include <unistd.h>
LWorkerThread::LWorkerThread()
{
	pthread_create(&this->m_pthreadId,NULL,this->worker,this);
	this->setDetach(true);
	pthread_detach(this->m_pthreadId);
	this->m_Job = nullptr;
	this->m_threadPool = nullptr;
}
LWorkerThread::~LWorkerThread()
{
	this->Terminate();
}

void LWorkerThread::Terminate()
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

void LWorkerThread::Start()
{
	this->setStart(true);
}

void* LWorkerThread::worker(void *arg)
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
	std::cout<<"WorkerThread Terminate" << std::endl;
	return NULL;
}

void LWorkerThread::setWorkId(unsigned int workId)
{
	this->m_workId = workId;
}

void LWorkerThread::setLJob(LJob* job)
{
	if(job == nullptr) return ;
	{
		//std::cout << "setjob =======" << endl;
		std::lock_guard<std::mutex> guard(m_varLocker);
		//std::cout << "require lock" << endl;
		this->m_Job = job;
	}
	m_isJobCond.notify_all();
	//std::cout << "unlock" << endl;
}
void LWorkerThread::setThreadPool(LThreadPool* threadpool)
{
	if(threadpool == nullptr) return ;
	{
		//std::cout << "setThreadPool =======" << endl;
		std::lock_guard<std::mutex> guard(m_varLocker);
		//std::cout << "require lock" << endl;
		this->m_threadPool = threadpool;
	}
	std::cout << "unlock" << endl;
}

//执行任务
void LWorkerThread::Run()
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
