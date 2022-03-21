/**
 * @file Acceptor.h
 * @author HYT
 * @date 2021.11.17
 * @brief Accept new connections only
 */
#ifndef _ACCEPTOR_H
#define _ACCEPTOR_H
#include <iostream>
#include <functional>
#include "socketimpl.h"
using namespace socketfactory;
namespace LightOi
{
	class Acceptor{
		typedef std::function<void(SocketImpl*&)> NewConnectCallback;
	public:
		Acceptor(){}
		
		~Acceptor(){}
	public:
		void setNewConnectCallbackFun(NewConnectCallback cb) { _newConcb = cb; }
		
		void handleAccept(ServerSocketImpl*& serverSok,int acceptnumber);
	private:
		NewConnectCallback _newConcb;
	};
}
#endif