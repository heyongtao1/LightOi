#ifndef _BLOG_H
#define _BLOG_H
#include "Message.h"
#include "Buffer.h"
#include "util.h"
#include "user.h"
namespace HYT{
	class Blog : public User{
	public:
		Blog() : buffer{10240}{}
		~Blog(){}
		
	public: // 接口
		bool read();
		
		bool write();
	
		void process();
	
	private:
		net::Buffer buffer;
		
		Message message;
	};
}
#endif