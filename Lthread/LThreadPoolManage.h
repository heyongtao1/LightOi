#ifndef _LTHREADPOOLMANAGE_H
#define _LTHREADPOOLMANAGE_H
#include "../User/blog.h"
#include "LThreadPool.h"

template <typename T>
class LThreadPoolManage{
public:
	LThreadPoolManage(int initThreadNum = 50)
	{
		m_threadpool = new LThreadPool<T>(initThreadNum);
	}
	~LThreadPoolManage()
	{
		if(m_threadpool != nullptr)
		{
			delete m_threadpool;
			m_threadpool = nullptr;
		}
	}

public:
	void addTask(T* job)
	{
		m_threadpool->Run(job);
	}
	
private:
	LThreadPool<T>* m_threadpool;
};

#endif