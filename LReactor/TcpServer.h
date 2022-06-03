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
#include "../User/blog.h"
#include "../LMysql/connect_pool.h"
#include "../Llib/Logger.h"
#include "../LSocket/socketimpl.h"
#define  NUMBER 30
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
		/*all modular start function*/
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
		/*Callback function disPatch new Connect*/
		void disPatchNewConnect(SocketImpl*& clientSok);
		
		void printTestInfo()
		{ _pool.printTotalActiveNumber(); }
	private:
		// Only responsible for new customer connection events
		MainReactor _mainReactor;
		/*In addition to connection events, it is only responsible for readable, 
		writable and exception events */
		SubReactorThreadPool<HYT::LJob> _pool;
	};
}
#endif