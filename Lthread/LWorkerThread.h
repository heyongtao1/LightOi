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
    THREAD_RUNNING = 0x00, //运行
    THREAD_IDLE = 0x01,//空闲
    THREAD_EXIT = 0X02,//退出
}ThreadState;

template <typename T> class LThreadPool;

template <typename T>
class LWorkerThread : public LThread<T> {
public:
	LWorkerThread()
	{

	}
	~LWorkerThread()
	{

	}
private:

public:
	void Start()
	{
		this->setStart(true);
		state = ThreadState::THREAD_RUNNING;
	}

private:
	//任务编号
	unsigned int m_workId;

	ThreadState state;
private:
	//任务的条件变量，等待执行任务和唤醒执行任务
	std::condition_variable m_isJobCond;
public:
	void setWorkId(unsigned int workId)
	{
		this->m_workId = workId;
	}
};

#endif