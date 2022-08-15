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
#include <memory>
#include <iostream>
#include "SubReactorThreadPoll.h"
//#include "http_conn.h"
#include "User/LJob.h"
#include "User/UDP_user.h"
#include "LMysql/connect_pool.h"
#include "Logger/Logger.h"
#include "LSocket/socketimpl.h"
#include "common_component/debug/LDebug.h"
#include "config.hpp"
using namespace socketfactory;
namespace LightOi
{
	class TcpServer{
	public:
		TcpServer(const char *address, uint16_t port)
		: _mainReactor{address,port}
		{
			_mainReactor.setdisPatchCallbackFun(std::bind(&TcpServer::disPatchNewConnect,this,std::placeholders::_1));
			_pool = std::make_shared<SubReactorThreadPool<HYT::LJob,UDP_user>>();
		}
		~TcpServer()
		{
			std::cout << "~TcpServer" << std::endl;
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
			_pool->stopTotalSubReactor();
			LogInfo(NULL);
			Logger::GetInstance().Stop();
		}
		/*Callback function disPatch new Connect*/
		void disPatchNewConnect(SocketImpl*& clientSok);
		
		void printTestInfo()
		{
			_pool->printTotalActiveNumber(); 
		}
	private:
		// Only responsible for new customer connection events
		MainReactor _mainReactor;
		/*In addition to connection events, it is only responsible for readable, 
		writable and exception events */
		//SubReactorThreadPool<HYT::LJob,UDP_user> _pool;
		std::shared_ptr<SubReactorThreadPool<HYT::LJob,UDP_user>> _pool;
	};
}
#endif