#include "blog.h"
#include "Logger.h"
namespace HYT{
	bool Blog::read()
	{
		LogInfo(NULL);
		int savedErrno = 1;
		int ret = buffer.readFd(clientSok,&savedErrno);
		if( savedErrno == errno || ret < 0)
		{
			return false;
		}
		return true;
	}
	
	bool Blog::write()
	{
		LogInfo(NULL);
		int savedErrno = 1;
		int ret = buffer.writeFd(clientSok,&savedErrno);
		if( savedErrno == errno || ret == 0)
		{
			return false;
		}
		else
		//恢复可读状态
		readable();
		return true;
	}
	
	void Blog::process()
	{
		LogInfo(NULL);
		//获取包头,包体为int的包体长度
		int head = buffer.readPackageHead();
		//获取包体
		const char* data = buffer.readPackageBody(head);
		char* ret_data = message.messageAnalysis(data);
		if(ret_data == NULL)
		{
			//恢复可读状态
			readable();
			return ;
		}
		//将解析后的数据写入缓冲区
		int ret_head = strlen(ret_data);
		buffer.append((char*)&ret_head,sizeof(ret_head));
		buffer.append(ret_data,ret_head);
		//通知主程序该fd可写，触发写事件
		writable();
	}

}