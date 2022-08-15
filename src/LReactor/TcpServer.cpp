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
		startLogger();
		startMySqlConnectPool();
		/* 阻塞函数 */
		startMainReactor(); 
	}
	void TcpServer::disPatchNewConnect(SocketImpl*& clientSok)
	{
		_pool->addNewConnectToSubReactor(clientSok);
	}
	
	void TcpServer::startLogger()
	{
		LDebug::ldebug("Logger start success");
		Logger::GetInstance().SetFileName(LOG_COUT_PATH);
		Logger::GetInstance().Start();
	}
	
	void TcpServer::startMySqlConnectPool()
	{
		//创建连接池中的连接个数
		vector<std::shared_ptr<MysqlHelper>> m_databases;
		for(int i=0;i<MYSQL_CONNECT_NUMBER;i++){
			std::shared_ptr<MysqlHelper> mysqlHelper = std::make_shared<MysqlHelper>();
			mysqlHelper->init("172.22.63.3","hyt","123456","Note","utf8");//uniLJob
			mysqlHelper->connect();
			m_databases.push_back(mysqlHelper);
		}
		
		if(!Singleton<connect_pool>::getInstance().init(m_databases)){
			LDebug::ldebug("connect_pool init fail");
		}
		else
			LDebug::ldebug("mysql connect_pool start success");
		std::cout << "mysql connect all start success" << std::endl;
	}

}