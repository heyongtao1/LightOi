#include "Acceptor.h"
#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "../Llib/Logger.h"
#include "../util/Protect.h"
#include "../util/Singleton.h"
#include "../util/util.h"
namespace LightOi
{
	/*TCP内核的心跳包机制，减少关闭状态的close_wait出现*/
	/*在程序中表现为,当tcp检测到对端socket不再可
	用时(不能发出探测包,或探测包没有收到ACK的响应包),
	select会返回socket可读,并且在recv时返回-1,
	同时置上errno为ETIMEDOUT.*/
	static void setSocketOpt(int fd)
	{
		int iKeepAlive = 1;// 开启keepalive属性
		int KeepAliveProbes=3;// 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
		int KeepAliveIntvl=5;// 探测时发包的时间间隔为5 秒
		int KeepAliveTime=60; // 如该连接在120秒内没有任何数据往来,则进行探测
		/*首先激活SO_KEEPALIVE*/
		setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&iKeepAlive, sizeof(iKeepAlive));
		
		setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&KeepAliveProbes, sizeof(KeepAliveProbes));
		setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&KeepAliveTime, sizeof(KeepAliveTime));
		setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&KeepAliveIntvl, sizeof(KeepAliveIntvl));
	}
	void Acceptor::handleAccept(ServerSocketImpl*& serverSok,int acceptnumber)
	{
		for(int i=0;i<acceptnumber;i++)
		{
			SocketImpl* client = serverSok->accept();
			char* clientIp = epoll_util::IP_tostring(client->address);
			
			if(Singleton<Protect>::getInstance().isExistence(string(clientIp)))
			{
				cout << "黑名单 ip " <<endl;
				SocketFactory::destroy(client);
				continue;
			}
			
			LogRun("%s",clientIp);
			
			if(client->fd < 0)
			{
				printf("errno is %d\n",errno);
				SocketFactory::destroy(client);
				continue;
			}
			setSocketOpt(client->fd);
			printf("客户端fd = %d上线\n",client->fd);
			_newConcb(client);
		}
	}

}