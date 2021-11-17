/**
 * @file TcpServer.h
 * @author HYT
 * @date 2021.11.17
 * @brief Enter the port and address to turn on the server
 */
#ifndef _TCPSERVER_H
#define _TCPSERVER_H
#include "SubReactorThreadPoll.h"
#include <functional>

#include "http_conn.h"
#include "threadpool.h"
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
		
		void startMainReactor() { _mainReactor.loop(); } 
		
		void stopMainReactor() { _mainReactor.stopLoop(); }
		
		void disPatchNewConnect(int NewConnfd);
	private:
		// Only responsible for new customer connection events
		MainReactor _mainReactor;
		/*In addition to connection events, it is only responsible for readable, 
		writable and exception events */
		SubReactorThreadPool<http_conn> _pool;
	};
}
#endif