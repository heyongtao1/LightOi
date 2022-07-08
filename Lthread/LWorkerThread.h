#ifndef _LWORKERTHREAD_H
#define _LWORKERTHREAD_H
#include "LThread.h"
#include "../User/blog.h"
#include "Llock.h"

using namespace HYT;

//线程状态
typedef enum _ThreadState
{
    THREAD_RUNNING = 0x0, //运行
    THREAD_IDLE = 0x1,//空闲
    THREAD_EXIT = 0X2,//退出
}ThreadState;

class LThreadPool;

class LWorkerThread : public LThread {
public:
	LWorkerThread();
	~LWorkerThread();
private:
	static void* worker(void *arg);
public:
	//接口
	void Run();
	void Start();
	void Terminate();
private:
	//执行的任务
	LJob* m_Job;
	//所在的线程池对象
	LThreadPool* m_threadPool;
	//任务编号
	unsigned int m_workId;
private:
	//任务的条件变量，等待执行任务和唤醒执行任务
	LThreadCond m_isJobCond;
	//对于设置任务、设置线程池对象的锁
	LThreadlocker m_varLocker;
public:
	//任务执行的锁，防止多个线程执行任务
	LThreadlocker m_workLocker;
public:
	void setLJob(LJob* job);
	void setThreadPool(LThreadPool* threadpool);
	void setWorkId(unsigned int workId);
};

#endif