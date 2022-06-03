/*
* socketfactory.h
*
*  Created on: 2014-7-19
*      Author: root
*/

#ifndef SOCKETFACTORY_H_
#define SOCKETFACTORY_H_
#include<sys/types.h>
 
/*
* 在网路编程中， 一般分为服务端和客户端，两者的行为有相似之处，也有非常多的不同。在linux中对socket程序设计
* 仅提供了socket(), bind(), connection(), accept() listern() 几个函数，并未区分是服务端还是客户端。
* 在java或其他高级语言中，一般都对socket进行了封装，简化使用。
*  此端程序将linux socket进行封装在三个类中。一个工厂类，两个接口类。
*
*/
 
namespace socketfactory {
 
class ISocket;
class IServerSocket;
 
/*
* SocketFactory 负责创建 ISocket， IServerSocket 的实例，同时负责销毁实例。
* SocketFactory的方法全部是静态方法。此处采用了工厂模式。
*
*/
class SocketFactory {
public:
     static ISocket* createSocket(const char* tIP, int tPort);    /* 创建客户端 socket */
     static IServerSocket* createServerSocket(int port);            /* 创建服务端 socket */
     static int destroy(ISocket* psocket);                                /* 销毁 */
     static int destroy(IServerSocket* psocket);                        /* 销毁 */
};
 
/*
* ISocket 为接口，定义为纯虚类。面向接口编程
*
*/
 
class ISocket {
public:
     virtual int read(void* buf, size_t len)=0;        /*    读取对端数据 */
     virtual int write(void* buf, size_t len)=0;        /* 写入对端数据 */
     virtual int close()=0;                                    /* 关闭连接 */
	
};
 
/*
* IServerSocket 为接口，定义为纯虚类。面向接口编程。
*
*/
 
class IServerSocket {
 public:
     virtual ISocket* accept()=0;                        /* 接受连接，返回与对端通信的socket */
     virtual int listen(int backlog)=0;                /*    启动服务端口 监听 */
     virtual int close()=0;                                /* 关闭 服务端 socket */
};
 
}

#endif /* SOCKETFACTORY_H_ */