#include <iostream>
#include <memory>
#include "TcpServer.h"
using namespace std;


int main(int argc, char **argv)
{
	LightOi::TcpServer server("192.168.206.129", 8888);
	server.start();
	return 0;
}