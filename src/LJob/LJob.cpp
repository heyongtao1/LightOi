#include "LJob.h"
#include "Logger/Logger.h"
#define  STRING_TEST "hello world!"
namespace HYT{
	bool LJob::read()
	{
		LOGINFO(NULL);
		int savedErrno = 1;
		int ret = buffer->readFd(clientSok,&savedErrno);
		if( savedErrno == errno || ret < 0) return false;
		return true;
	}
	
	bool LJob::write()
	{
		LOGINFO(NULL);
		int savedErrno = 1;
		int ret = buffer->writeFd(clientSok,&savedErrno);
		if( savedErrno == errno || ret == -1) return false;
		readable();
		return true;
	}
	
	void LJob::task()
	{
		LOGINFO(NULL);
		int head = buffer->readPackageHead();
		auto data = buffer->readPackageBody(head);
		std::string result;
		{
			#ifdef 	 ENABLE_MYSQL
				result = Singleton<Message>::getInstance().messageAnalysis(data);
			#else
				result = STRING_TEST;
			#endif	
		}

		if(result.empty())
		{
			LOGINFO(NULL);
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
			if(clientSok->send_data((void*)&ret_head,sizeof(ret_head)) == -1) LOGERROR(NULL);
			if(clientSok->send_data(datas,strlen(datas)) == -1) LOGERROR(NULL);
			readable();
		}
#endif
	}

}
