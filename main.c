#include <iostream>
#include <memory>
#include <csignal>
#include "Logger.h"
#include "TcpServer.h"
using namespace std;

LightOi::TcpServer server("172.22.63.3", 9999);

void term(int signal) {
	//server.printTestInfo();
	LogInfo(NULL);
	server.stop();
}

int main(int argc, char **argv)
{
	signal(SIGINT, term);
	server.start();
	return 0;
}
