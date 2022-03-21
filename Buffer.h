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
#include "Logger.h"
#include "socketimpl.h"
#include "Protect.h"
#include "Singleton.h"
#include "util.h"
using namespace socketfactory;
using namespace std;

namespace net{

typedef struct packageHead{
	char confirm[3] = {0};
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
	  buffer_.capacity();
	}
	// Read data directly into buffer.
	ssize_t readFd(SocketImpl*& clientSok, int* savedErrno)
	{
		LogInfo(NULL);
		int hasRecv = 0;
		pkgH mpkgH;
		char extrabuff[65536];
		int ret;
		//先接收确认值
		ret = recv(clientSok->fd,mpkgH.confirm,sizeof(mpkgH.confirm),0);
		//客户端已关闭连接
		if(ret == -1) return -1;
		//确认值错误，则关闭连接
		if(strcmp(mpkgH.confirm,"HYT")!= 0 && strcmp(mpkgH.confirm,"TST")!= 0)
		{
			LogRun("%s,data:%s","recv confirm fail return -1",mpkgH.confirm);
			//加入黑名单，外来非法连接
			char* clientIp = epoll_util::IP_tostring(clientSok->address);
			Singleton<Protect>::getInstance().insertBlacklistSet(string(clientIp));
			return -1;
		}
		if(strcmp(mpkgH.confirm,"TST") == 0)
		{
			mpkgH.len = 955;
		}
		else
		{
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
		//cout << "packageHead = " << mpkgH.len << endl;
		while(mpkgH.len > hasRecv)
		{
			const size_t writable = writableBytes();
			struct iovec vec[2];
			memset(extrabuff,0,sizeof(extrabuff));
			vec[0].iov_base = begin() + writerIndex_;
			vec[0].iov_len = writable;
			vec[1].iov_base = extrabuff;
			vec[1].iov_len = sizeof extrabuff;
			const ssize_t n = readv(clientSok->fd,vec,2);
			if(n < 0)
			{
				usleep(200);
			}else if(n <= writable)
			{
				writerIndex_ += n;
				hasRecv += n;
			}else
			{
				writerIndex_ = buffer_.size();
				append(extrabuff, n - writable);
				hasRecv += n;
			}
		}
		if(mpkgH.len != hasRecv)
		{
			cout << "mpkgH.len = " << mpkgH.len << " hasRecv = " << hasRecv <<endl;		
			cout << "read len  = " << readableBytes() << endl;
			LogRun("%s","recv mpkgH.len != hasRecv fail return -1");
			*savedErrno = errno;
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
		int ret;
again:
		if((ret = send(clientSok->fd,(char*)&headLen,sizeof(headLen),0)) == 0)
		{
			return 0;
		}
		else if(ret < 0)
		{
			if(errno == EINTR ||errno == EAGAIN ||errno == EWOULDBLOCK)
			goto again;
		}
		while(headLen > hasWrite)
		{
			const size_t readable = readableBytes();
			vec[0].iov_base = begin() + readerIndex_;
			vec[0].iov_len = readable;
			const ssize_t n = writev(clientSok->fd,vec,1);
			if(n < 0)
			{
				usleep(200);
			}else if(n <= readable)
			{
				readerIndex_ += n;
				hasWrite += n;
			}
		}
		if(headLen != hasWrite) *savedErrno = errno;
		return hasWrite;
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
		  cout << "FIXME" << endl;
		  buffer_.resize(writerIndex_+len);
		}
		else
		{
		  // move readable data to the front, make space inside buffer
		  cout << "move" << endl;
		  assert(kCheapPrepend < readerIndex_);
		  size_t readable = readableBytes();
		  std::copy(begin()+readerIndex_,
					begin()+writerIndex_,
					begin()+kCheapPrepend);
		  readerIndex_ = kCheapPrepend;
		  writerIndex_ = readerIndex_ + readable;
		  assert(readable == readableBytes());
		}
		cout << "readable bytes = " << readableBytes() << endl;
		cout << "writeable bytes = " << writableBytes() << endl;
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