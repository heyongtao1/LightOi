#ifndef _LTHREADPOOLMANAGE_H
#define _LTHREADPOOLMANAGE_H
#include <memory>
#include "User/LJob.h"
#include "LThreadPool.h"

template <typename T>
class LThreadPoolManage{
public:
	LThreadPoolManage(int initThreadNum = 50)
	{
		m_threadpool = std::make_shared<LThreadPool<T>>(initThreadNum);
	}
	~LThreadPoolManage(){}

public:
	void addTask(T* job)
	{
		m_threadpool->Run(job);
	}
	
private:
	std::shared_ptr<LThreadPool<T> > m_threadpool;
};

#endif