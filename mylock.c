#include "mylock.h"

//创建信号量
sem :: sem()
{
    if(sem_init(&m_sem,0,0) != 0)
    {
        throw std ::exception();
    }
}
//销毁信号量
sem :: ~sem()
{
    sem_destroy(&m_sem);
}
//等待信号量
bool sem::wait()
{
    return sem_wait(&m_sem) == 0;
}
//增加信号量
bool sem::post()
{
    return sem_post(&m_sem) == 0;
}

mylocker::mylocker()
{
    if(pthread_mutex_init(&m_mutex, NULL) != 0)
    {
        throw std::exception();
    }
}
 
mylocker::~mylocker()
{
    pthread_mutex_destroy(&m_mutex);
}
/*上锁*/
bool mylocker::lock()
{
    return pthread_mutex_lock(&m_mutex)==0;
}
/*解除锁*/
bool mylocker::unlock()
{
    return pthread_mutex_unlock(&m_mutex) == 0;
}

mycond::mycond()
{
    if(pthread_mutex_init(&m_mutex,NULL)!=0)
    {
        throw std::exception();
    }
    if(pthread_cond_init(&m_cond, NULL)!=0)
    {
        throw std::exception();
    }
}
 
mycond::~mycond()
{
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}
 
/*等待条件变量*/
bool mycond::wait()
{
    int ret;
    pthread_mutex_lock(&m_mutex);
    ret = pthread_cond_wait(&m_cond,&m_mutex);
    pthread_mutex_unlock(&m_mutex);
    return ret == 0;
}
 
/*唤醒等待条件变量的线程*/
bool mycond::signal()
{
    return pthread_cond_signal(&m_cond) == 0;
}