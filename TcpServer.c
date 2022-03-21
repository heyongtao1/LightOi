/**
 * @file TcpServer.c
 * @author HYT
 * @date 2021.11.17
 * @brief Enter the port and address to turn on the server
 */
#include "TcpServer.h"
//初始化连接池
connect_pool* connect_pool::m_connect_pool = new connect_pool();
connect_pool::deleteMysqlHelper connect_pool::deletepool;

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
		_pool.addNewConnectToSubReactor(clientSok);
	}
	
	void TcpServer::startLogger()
	{
		cout << "Logger start success" << endl;
		Logger::GetInstance().SetFileName("./log/server.log");
		Logger::GetInstance().Start();
	}
	
	void TcpServer::startMySqlConnectPool()
	{
		//创建连接池中的连接个数
		vector<MysqlHelper *> m_databases;
		for(int i=0;i<NUMBER;i++){
			printf("new MysqlHelper %d\n",i);
			MysqlHelper *mysqlHelper = new MysqlHelper();
			mysqlHelper->init("172.22.63.3","root","123456","uniblog","utf8");
			mysqlHelper->connect();
			m_databases.push_back(mysqlHelper);
		}
		
		if(!connect_pool::get_instance()->init(m_databases)){
			printf("connect_pool init fail\n");
		}
		else
			printf("mysql connect_pool start success\n");
	}

}