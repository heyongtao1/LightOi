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

class LThreadPool{
public:
	LThreadPool();
	LThreadPool(int initThreadNum);
	~LThreadPool();
public:	
	friend class LWorkerThread;
public:
	//将空闲线程加入到忙碌队列
	void moveIdleThreadToBusyQueue(LWorkerThread* idleThread);
	//将忙碌线程加入到空闲队列
	void moveBusyThreadToIdleQueue(LWorkerThread* busyThread);
	//获取空闲线程
	LWorkerThread* getIdleThread();
	//增加一个工作线程到空闲队列
	void appendWorkThreadToIdleQueue(LWorkerThread* workthread);
	//增加多个工作线程到空闲队列
	void createWorkThreadToIdleQueue(int num);
	//从空闲队列删除多个空闲线程
	void deleteWorkThreadFromIdleQueue(int num);

public:
    void TerminateAll();
    void Run(LJob* job);
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
	std::vector<LWorkerThread*> m_allThread;
	//空闲线程的队列
	std::vector<LWorkerThread*> m_idleThread;
	//忙碌线程的队列
	std::vector<LWorkerThread*> m_busyThread;
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