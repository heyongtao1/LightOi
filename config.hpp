#ifndef _CONFIG_H
#define _CONFIG_H
//监听事件大小
#define MAX_CONN_EVENT_NUMBER 10240
//工作线程池的大小
#define MAX_THREAD_NUMBER 5
//最大线程数
#define MAX_WTHREAD_NUMBER 10
//最小线程数
#define MIN_WTHREAD_NUMBER 5
//任务队列的最大值
#define MAX_WORK_NUMBER 100
//子Reactor的大小
#define SUBREACTOR_THREAD_MAX_NUM 5
//数据库连接的大小
#define  MYSQL_CONNECT_NUMBER 60

#define UDP_THREAD_NUM 1
//日志输出路径
#define LOG_COUT_PATH "../log/server.log"

#endif