#ifndef _THREADPOOL_H
#define _THREADPOOL_H
#include<iostream>
#include<list>
#include<cstdio>
#include<semaphore.h>
#include<exception>
#include<pthread.h>
#include"mylock.h"
#define MAX_PTHREAD_NUMBER 50
using namespace std;
 
template<typename T>
/*线程池的封装*/
class threadpool
{
private:
    int max_thread;//线程池中的最大线程总数
    int max_job;//工作队列的最大总数
    pthread_t *pthread_poll;//线程池数组
    std::list<T*> m_myworkqueue;//请求队列
    mylocker m_queuelocker;//保护请求队列的互斥锁
    sem m_queuestat;//由信号量来判断是否有任务需要处理
    bool m_stop;//是否结束线程
public:
    threadpool();
    ~threadpool();
    bool addjob(T* request);
private:
    static void* worker(void *arg);
    void run();
};
/*线程池的创建*/
template <typename T>
threadpool<T> :: threadpool()
{
    max_thread = MAX_PTHREAD_NUMBER;
    max_job = 1000;
    m_stop = false;
    pthread_poll = new pthread_t[max_thread];//为线程池开辟空间
    if(!pthread_poll)
    {
        throw std::exception();
    }
    for(int i=0; i<max_thread; i++)
    {
        cout << "Create the pthread:" << i << endl;
        if(pthread_create(pthread_poll+i, NULL, worker, this)!=0)
        {
            delete [] pthread_poll;
            throw std::exception();
        }
        if(pthread_detach(pthread_poll[i]))//将线程分离
        {
            delete [] pthread_poll;
            throw std::exception();
        }
    }
}
 
template <typename T>
threadpool<T>::~threadpool()
{
    delete[] pthread_poll;
    m_stop = true;
}
 
template <typename T>
bool threadpool<T>::addjob(T* request)
{
    m_queuelocker.lock();
    if(m_myworkqueue.size()> max_job)//如果请求队列大于了最大请求队列，则出错
    {
        m_queuelocker.unlock();
        return false;
    }
    m_myworkqueue.push_back(request);//将请求加入到请求队列中
    m_queuelocker.unlock();
    m_queuestat.post();//将信号量增加1
    return true;
}
//工作线程的执行任务函数
template <typename T>
void* threadpool<T>::worker(void *arg)
{
    threadpool *pool = (threadpool*)arg;
    pool->run();
    return pool;
}
 
template <typename T>
void threadpool<T> :: run()
{
    while(!m_stop)
    {
        m_queuestat.wait();//信号量减1，直到为0的时候线程挂起等待
        m_queuelocker.lock();//互斥锁上锁，防止对任务队列的竞态条件的竞争
        if(m_myworkqueue.empty())
        {
            m_queuelocker.unlock();//互斥锁解锁
            continue;
        }
        T* request = m_myworkqueue.front();
        m_myworkqueue.pop_front();
        m_queuelocker.unlock();
        if(!request)
        {
            continue;
        }
        request->process();//执行工作队列
    }
	
}
#endif