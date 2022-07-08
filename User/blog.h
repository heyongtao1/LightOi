#ifndef _LJob_H
#define _LJob_H
#include "Message.h"
#include "Buffer.h"
#include "../util/util.h"
#include "user.h"
namespace HYT{
	class LJob : public User{
	public:
		LJob() : buffer{10240}{}
		~LJob(){}
		
	public: // 接口
		bool read();
		
		bool write();
	
		void task();
	
	private:
		net::Buffer buffer;
		
		Message message;
	};
}
#endif