#include "LJob.h"
#include "Logger/Logger.h"
namespace HYT{
	bool LJob::read()
	{
		LogInfo(NULL);
		int savedErrno = 1;
		int ret = buffer->readFd(clientSok,&savedErrno);
		if( savedErrno == errno || ret < 0) return false;
		return true;
	}
	
	bool LJob::write()
	{
		LogInfo(NULL);
		int savedErrno = 1;
		int ret = buffer->writeFd(clientSok,&savedErrno);
		if( savedErrno == errno || ret == -1) return false;
		//恢复可读状态
		readable();
		return true;
	}
	
	void LJob::task()
	{
		LogInfo(NULL);
		//获取包头,包体为int的包体长度
		int head = buffer->readPackageHead();
		//获取包体
		auto data = buffer->readPackageBody(head);
		auto result = Singleton<Message>::getInstance().messageAnalysis(data);
		if(result == "")
		{
			//恢复可读状态
			LogInfo(NULL);
			readable();
			return ;
		}
		const char* datas = result.c_str();
		int ret_head = strlen(datas);
#ifdef ENABLE_WRITE
		{
			//将解析后的数据写入缓冲区
			buffer->append((char*)&ret_head,sizeof(ret_head));
			buffer->append(datas,ret_head);
			//通知主程序该fd可写，触发写事件
			writable();
		}
#else
		{
			//直接发送返回
			if(clientSok->send_data((void*)&ret_head,sizeof(ret_head)) == -1) LogError(NULL);
			if(clientSok->send_data(datas,strlen(datas)) == -1) LogError(NULL);
			readable();
		}
#endif
		delete data;
		data = NULL;
	}

}
