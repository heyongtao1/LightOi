#include "LThreadPoolManage.h"

LThreadPoolManage::LThreadPoolManage()
{
	m_threadpool = new LThreadPool(50);
}
LThreadPoolManage::LThreadPoolManage(int initThreadNum)
{
	m_threadpool = new LThreadPool(initThreadNum);
}
LThreadPoolManage::~LThreadPoolManage()
{
	if(m_threadpool != nullptr)
	{
		delete m_threadpool;
		m_threadpool = nullptr;
	}
}

void LThreadPoolManage::addTask(LJob* job)
{
	m_threadpool->Run(job);
}