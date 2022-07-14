#include "blog.h"
#include "../Llib/Logger.h"
namespace HYT{
	bool LJob::read()
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
	
	bool LJob::write()
	{
		LogInfo(NULL);
		int savedErrno = 1;
		int ret = buffer.writeFd(clientSok,&savedErrno);
		if( savedErrno == errno || ret == -1)
		{
			return false;
		}
		else
		//恢复可读状态
		readable();
		return true;
	}
	
	void LJob::task()
	{
		LogInfo(NULL);
		//获取包头,包体为int的包体长度
		int head = buffer.readPackageHead();
        LogInfo(NULL);
		//获取包体
		const char* data = buffer.readPackageBody(head);
        LogInfo(NULL);
        cout << "data = " << data << endl;
		char* ret_data = Singleton<Message>::getInstance().messageAnalysis(data);
        LogInfo(NULL);
		if(ret_data == NULL)
		{
			//恢复可读状态
			LogInfo(NULL);
			readable();
			return ;
		}
		int ret_head = strlen(ret_data);
		if(false)
		{
			//将解析后的数据写入缓冲区
			buffer.append((char*)&ret_head,sizeof(ret_head));
			buffer.append(ret_data,ret_head);
			//通知主程序该fd可写，触发写事件
			writable();
			return ;
		}
		else
		{
			//直接发送返回
			if(clientSok->send_data((void*)&ret_head,sizeof(ret_head)) == -1) LogError(NULL);
			if(clientSok->send_data(ret_data,strlen(ret_data)) == -1) LogError(NULL);
			readable();
		}
	}

}
