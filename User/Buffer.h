#ifndef BUFFER_H
#define BUFFER_H

#include <iostream>
#include <string.h>
#include <stdint.h>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <vector>
#include <sys/uio.h>
#include <sys/types.h>   
#include <sys/socket.h>
#include "../Llib/Logger.h"
#include "../LSocket/socketimpl.h"
#include "../util/Protect.h"
#include "../util/Singleton.h"
#include "../util/util.h"
using namespace socketfactory;
using namespace std;

namespace net{

typedef struct packageHead{
	char confirm[4] = {0};
	int  len = 0;
}pkgH;


class Buffer{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;
	enum RECV_STATUS{ PACKAGE_HEADER = 0 , PACKAGE_BODY};
public:
	Buffer(size_t initialSize = kInitialSize)
		:buffer_(kCheapPrepend + initialSize),
		 readerIndex_(kCheapPrepend),
		 writerIndex_(kCheapPrepend)
	{
		assert(readableBytes() == 0);
		assert(writableBytes() == initialSize);
		assert(prependableBytes() == kCheapPrepend);
	}
	~Buffer(){
		buffer_.clear();
		vector<char>().swap(buffer_);
	}
public:
	size_t readableBytes() const
	{ return writerIndex_ - readerIndex_; }

	size_t writableBytes() const
	{ return buffer_.size() - writerIndex_; }

	size_t prependableBytes() const
	{ return readerIndex_; }
	
	const char* peek() const
    { return begin() + readerIndex_; }
	
	const int readPackageHead()
	{
		int packageHead = 0;
		strncpy((char*)&packageHead,peek(),sizeof(packageHead));
		hasReaden(sizeof(packageHead));
		return packageHead;
	}
	
	const char* readPackageBody(int len)
	{
		char* str = (char*)malloc(sizeof(char)*(len+1));
		memset(str,'\0',len+1);
		strncpy(str,peek(),len);
		hasReaden(len);
		return str;
	}

	void append(const char* /*restrict*/ data, size_t len)
	{
		ensureWritableBytes(len);
		std::copy(data, data+len, beginWrite());
		hasWritten(len);
	}

	void append(const void* /*restrict*/ data, size_t len)
	{
		append(static_cast<const char*>(data), len);
	}

	void ensureWritableBytes(size_t len)
	{
		if (writableBytes() < len)
		{
		  makeSpace(len);
		}
		assert(writableBytes() >= len);
	}

	char* beginWrite()
	{ return begin() + writerIndex_; }

	const char* beginWrite() const
	{ return begin() + writerIndex_; }

	void hasWritten(size_t len)
	{
		assert(len <= writableBytes());
		writerIndex_ += len;
	}
	
	void hasReaden(size_t len)
	{
		//assert(len <= readableBytes());
		readerIndex_ += len;
	}
	
	size_t internalCapacity() const
	{
	  	return buffer_.capacity();
	}
	// Read data directly into buffer.
	ssize_t readFd(SocketImpl*& clientSok, int* savedErrno)
	{
		LogInfo(NULL);
		int hasRecv = 0;
		pkgH mpkgH;
		memset(mpkgH.confirm,'\0',sizeof(mpkgH.confirm));
		int ret;
		//先接收确认值
		if(recv(clientSok->fd,mpkgH.confirm,3,0) == -1) return -1;

		//确认值错误，则关闭连接
		if(strcmp(mpkgH.confirm,"HYT")!= 0 && strcmp(mpkgH.confirm,"TST")!= 0)
		{
			LogRun("recv confirm fail return -1,data:%s",mpkgH.confirm);
			//加入黑名单，外来非法连接
			char* clientIp = epoll_util::IP_tostring(clientSok->address);
			Singleton<Protect>::getInstance().insertBlacklistSet(string(clientIp));
			return -1;
		}
		if(strcmp(mpkgH.confirm,"TST") == 0)
		{
			LogInfo(NULL);
			mpkgH.len = 82;
		}
		else
		{
			LogInfo(NULL);
			again:
			if((ret = recv(clientSok->fd,(char*)&mpkgH.len,sizeof(mpkgH.len),0)) == 0)
			{
				LogRun("%s","recv mpkgH.len fail return -1");
				return -1;
			}
			else if(ret < 0)
			{
				if(errno == EINTR ||errno == EAGAIN ||errno == EWOULDBLOCK)
				goto again;
				else // errno == ETIMEDOUT
				return -1;
			}		
		}

		append((char*)&mpkgH.len,sizeof(mpkgH.len));

		{
			//使用新接口读取数据
			ensureWritableBytes(mpkgH.len);
			if(clientSok->recv_data(buffer_.data()+writerIndex_,mpkgH.len) == -1) return -1; 
			hasWritten(mpkgH.len);
		}

		return mpkgH.len;
	}
	
	// Write data directly into socketclientSok->fd from buffer;
	ssize_t writeFd(SocketImpl*& clientSok, int* savedErrno)
	{
		LogInfo(NULL);
		struct iovec vec[1];
		int hasWrite = 0;
		int headLen = readPackageHead();

        {
			if(readableBytes() < headLen) return 0;
            auto data = readPackageBody(headLen);
			cout << "send size = " << headLen <<endl;
            cout << "send data = " << data <<endl;
			if(clientSok->send_data((void *)&headLen,sizeof(headLen)) == -1) return -1;
			if(clientSok->send_data(data,headLen) == -1) return -1;
            LogInfo(NULL);
			delete data;
			data = NULL;
			return headLen;
		}
	    	
	}
private:
	char* begin()
	{ return &*buffer_.begin(); }

	const char* begin() const
	{ return &*buffer_.begin(); }

	void makeSpace(size_t len)
	{
		LogInfo(NULL);
		if (writableBytes() + prependableBytes() < len + kCheapPrepend)
		{
		  // FIXME: move readable data
		  buffer_.resize(writerIndex_+len);
		}
		else
		{
		  // move readable data to the front, make space inside buffer
		  assert(kCheapPrepend < readerIndex_);
		  size_t readable = readableBytes();
		  std::copy(begin()+readerIndex_,
					begin()+writerIndex_,
					begin()+kCheapPrepend);
		  readerIndex_ = kCheapPrepend;
		  writerIndex_ = readerIndex_ + readable;
		  assert(readable == readableBytes());
		}
	}
private:
	/* 缓冲区当前的状态机 */
	RECV_STATUS curr_Status = PACKAGE_HEADER;
	/* 缓冲区 */
	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;
};

}
#endif
