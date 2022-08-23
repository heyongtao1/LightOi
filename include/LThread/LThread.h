#ifndef _LTHREAD_H
#define _LTHREAD_H
#include <pthread.h>
template <class T>
class LThread{
public:
	LThread(bool isdetach = false):m_isStart(false),m_isTerminate(false),m_isDetach(isdetach){}
	virtual ~LThread(){}
public:	
	//设置是否启动线程
	void setStart(bool isStart){ m_isStart = isStart; }
	bool getStart(){ return m_isStart; }
	//设置是否线程终止
	void setTerminate(bool isTerminate){ m_isTerminate = isTerminate; }
	bool getTerminate(){ return m_isTerminate; }
	//设置是否线程分离
	void setDetach(bool isDetach){ m_isDetach = isDetach; }
	bool getDetach(){ return m_isDetach; }
	//获取线程id
	pthread_t getLThreadID(){ return m_pthreadId; }
public://定义接口
	//启动线程
    virtual void Start() = 0;       
private:
	//是否启动线程
	bool m_isStart;
	//是否终止线程
	bool m_isTerminate;
	//是否线程分离
	bool m_isDetach;
public:
	//线程id
	pthread_t m_pthreadId;  
};

#endif