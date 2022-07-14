#include <iostream>
#include <memory>
#include <csignal>
#include "Llib/Logger.h"
#include "LReactor/TcpServer.h"
#include <unistd.h>
using namespace std;

LightOi::TcpServer server("172.22.63.3", 10001);

void term(int signal) {
	//server.printTestInfo();
	LogInfo(NULL);
	server.stop();
	sleep(5);
}

int main(int argc, char **argv)
{
	signal(SIGINT, term);
	server.start();
	return 0;
}
