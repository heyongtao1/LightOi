/**
 * @file TcpServer.c
 * @author HYT
 * @date 2021.11.17
 * @brief Enter the port and address to turn on the server
 */
#include "TcpServer.h"
namespace LightOi
{
	void TcpServer::start()
	{
		 startMainReactor(); 
	}
	void TcpServer::disPatchNewConnect(int NewConnfd)
	{
		_pool.addNewConnectToSubReactor(NewConnfd);
	}
}