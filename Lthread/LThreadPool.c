#include "LThreadPool.h"

LThreadPool::LThreadPool()
{
	m_realIdleNum = m_initThreadNum = 50;
	m_maxThreadNum = 100;
	m_maxIdleNum = m_maxThreadNum - 10;
	m_minIdleNum = 5;
	
	m_allThread.clear();
	m_idleThread.clear();
	m_busyThread.clear();
	
	this->m_workId = 0;
	
	createWorkThreadToIdleQueue(m_initThreadNum);
}

LThreadPool::LThreadPool(int initThreadNum)
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

LThreadPool::~LThreadPool()
{
	TerminateAll();
	m_allThread.clear();
	m_idleThread.clear();
	m_busyThread.clear();
}

void LThreadPool::TerminateAll()
{
    for (int i=0; i<m_allThread.size(); ++i)
    {
		delete m_allThread[i];
		m_allThread[i] = nullptr;
    }
	std::cout << "m_allThread.size() = " << m_allThread.size()<< std::endl;
	std::cout << "realuseThread = " << m_idleThread.size() + m_busyThread.size()<< std::endl;
}
//获取空闲线程
LWorkerThread* LThreadPool::getIdleThread()
{
	std::unique_lock<std::mutex> gurad(m_idlequeLock);
	while(m_idleThread.size() == 0)
	{
		m_idlequeCond.wait(gurad);
	}
	
	if(m_idleThread.size() > 0)
	{
		LWorkerThread* idlethread = m_idleThread.front();
		gurad.unlock();
		return idlethread;
	}
	
	//获取失败
	gurad.unlock();
	return nullptr;
}
//加入一个工作线程到空闲线程
void LThreadPool::appendWorkThreadToIdleQueue(LWorkerThread* workthread)
{
	if(workthread == nullptr) return ;
	{
		std::lock_guard<std::mutex> gurad(m_idlequeLock);
		m_allThread.push_back(workthread);
		m_idleThread.push_back(workthread);
	}
}
//将空闲线程加入到忙碌线程
void LThreadPool::moveIdleThreadToBusyQueue(LWorkerThread* idleThread)
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
//将忙碌线程加入到空闲线程
void LThreadPool::moveBusyThreadToIdleQueue(LWorkerThread* busyThread)
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
//创建工作线程
void LThreadPool::createWorkThreadToIdleQueue(int num)
{
	std::cout << "LThreadPool::createWorkThreadToIdleQueue num = " << num << std::endl;
	{
		std::lock_guard<std::mutex> gurad(m_idlequeLock);
		for(int i=0;i<num;i++)
		{
			LWorkerThread* workthread = new LWorkerThread();
			workthread->setThreadPool(this);
			
			m_allThread.push_back(workthread);
			m_idleThread.push_back(workthread);
			
			workthread->Start();
		}
	}
	
	m_idlequeCond.notify_all();
	m_maxThreadNumCond.notify_all();
}
//销毁工作线程
void LThreadPool::deleteWorkThreadFromIdleQueue(int num)
{
	std::lock_guard<std::mutex> gurad(m_idlequeLock);
	for(int i=0;i<num;i++)
	{
		LWorkerThread* deltThread;
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
//线程调度接口
void LThreadPool::Run(LJob* job)
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
	LWorkerThread* workThread = getIdleThread();
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