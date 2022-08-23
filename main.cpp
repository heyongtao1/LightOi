#include <iostream>
#include <memory>
#include <csignal>
#include "Logger/Logger.h"
#include "LReactor/TcpServer.h"
#include <unistd.h>
using namespace std;

shared_ptr<LightOi::TcpServer> server = make_shared<LightOi::TcpServer>("172.22.63.3", 9999);

void term(int signal) {
	//server.printTestInfo();
	LOGINFO(NULL);
	server->stop();
	sleep(5);
}

int main(int argc, char **argv)
{
	signal(SIGINT, term);
	server->start();
	return 0;
}
