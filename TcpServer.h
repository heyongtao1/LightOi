/**
 * @file TcpServer.h
 * @author HYT
 * @date 2021.11.17
 * @brief Enter the port and address to turn on the server
 */
#ifndef _TCPSERVER_H
#define _TCPSERVER_H
#include <functional>
#include <stdio.h>
#include "SubReactorThreadPoll.h"
//#include "http_conn.h"
#include "Logger.h"
#include "blog.h"
#include "threadpool.h"
#include "connect_pool.h"
#include "Logger.h"
#include "socketimpl.h"
#define  NUMBER 100
using namespace socketfactory;
namespace LightOi
{
	class TcpServer{
	public:
		TcpServer(const char *address, uint16_t port)
		: _mainReactor{address,port}
		{
			_mainReactor.setdisPatchCallbackFun(std::bind(&TcpServer::disPatchNewConnect,this,std::placeholders::_1));
		}
	
	public:
		void start();
		/* mysql connect pool start*/
		void startMySqlConnectPool();
		/* Logger work thread start*/
		void startLogger();
		/* mainReactor loop start*/
		void startMainReactor() { _mainReactor.loop(); } 
		
		void stop() 
		{ 
			_mainReactor.stop(); 
			_pool.stopTotalSubReactor();
			LogInfo(NULL);
			Logger::GetInstance().Stop();
		}
		
		void disPatchNewConnect(SocketImpl*& clientSok);
		
		void printTestInfo()
		{ _pool.printTotalActiveNumber(); }
	private:
		// Only responsible for new customer connection events
		MainReactor _mainReactor;
		/*In addition to connection events, it is only responsible for readable, 
		writable and exception events */
		SubReactorThreadPool<HYT::Blog> _pool;
	};
}
#endif