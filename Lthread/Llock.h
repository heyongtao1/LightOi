#ifndef _MYLOCK_H
#define _MYLOCK_H
#include<semaphore.h>
#include<exception>
#include<pthread.h>
 
/*封装信号量*/
class LThreadSem{
private:
    sem_t m_sem;
public:
    LThreadSem();
    ~LThreadSem();
    bool wait();//等待信号量
    bool post();//增加信号量
};
 
/*封装互斥锁*/
class LThreadlocker{
private:
    pthread_mutex_t m_mutex;
public:
    LThreadlocker();
    ~LThreadlocker();
    bool lock();
    bool unlock();
};
 
/*封装条件变量*/
class LThreadCond{
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
public:
    LThreadCond();
    ~LThreadCond();
    bool wait();
    bool signal();
};
 
#endif