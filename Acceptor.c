#include "Acceptor.h"
#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
namespace LightOi
{
	void Acceptor::handleAccept(int listenfd,int acceptnumber)
	{
		for(int i=0;i<acceptnumber;i++)
		{
			struct sockaddr_in client_address;
			socklen_t client_addresslength = sizeof(client_address);
			int client_fd = accept(listenfd,(struct sockaddr*)&client_address, &client_addresslength);
			if(client_fd < 0)
			{
				printf("errno is %d\n",errno);
				continue;
			}
			_newConcb(client_fd);
		}
	}

}