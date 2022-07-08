#ifndef _LTHREADPOOLMANAGE_H
#define _LTHREADPOOLMANAGE_H
#include "../User/blog.h"
#include "LThreadPool.h"

class LThreadPoolManage{
public:
	LThreadPoolManage();
	LThreadPoolManage(int initThreadNum);
	~LThreadPoolManage();

public:
	void addTask(LJob* job);
	
private:
	LThreadPool* m_threadpool;
};

#endif