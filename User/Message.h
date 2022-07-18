#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include "../Llib/cJSON.h"
#include "../LMysql/mysqlhelper.h"
#include "../Llib/base64.h"
using namespace std;
class Message{
public:
	char* messageAnalysis(const char* message);
	typedef enum {
		KEEPLIVE_MAG = 0x00,
		KEEPLIVE_SUCCESS = 0x01,
		KEEPLIVE_FAIL = 0X02
	}KeepLiveType;

	typedef enum {
		LOGIN_MAG = 0x10,
		LOGIN_SUCCESS = 0x11,
		LOGIN_FAIL = 0X12
	}LoginMagType;

	typedef enum {
		REGIST_MAG = 0x20,
		REGIST_SUCCESS = 0x21,
		REGIST_FAIL = 0X22
	}RegistMagType;

	typedef enum {
		BLOG_MAG = 0x30,
		BLOG_SUCCESS = 0x31,
		BLOG_FAIL = 0x32
	}BlogMagType;
	
	typedef enum {
		LOADBLOG_MAG = 0x40,
		LOADBLOG_SUCCESS = 0x41,
		LOADBLOG_FAIL = 0x42
	}LoadBlogMagType;

	typedef enum{
		FINDBLOG_MAG = 0X50,
		FINDBLOG_SUCCESS = 0X51,
		FINDBLOG_FAIL = 0X52
	}FindBlogMagType;

	typedef enum{
		RECOMBLOG_MAG = 0X60,
		RECOMBLOG_SUCCESS = 0X61,
		RECOMBLOG_FAIL = 0X62
	}RecomBlogMagType;

	typedef enum{
		FILE_MAG = 0X70,
		FILE_SUCCESS = 0X71,
		FILE_FAIL = 0X72
	}FileMsgTyPe;

	typedef enum{
		FILE_TYPE_PICTURE = 0X80,
		FILE_TYPE_TXT = 0X81,
		FILE_TYPE_MUSIC = 0X82,
		FILE_TYPE_VIDEO = 0X83,
		FILE_TYPE_OTHERFILE = 0X84
	}FileType;

	typedef enum{
		FILE_DOWNLOAD_MAG = 0X90,
		FILE_DOWNLOAD_SUCCESS = 0X91,
		FILE_DOWNLOAD_FAIL = 0X92
	}FILEDownLoadType;

	typedef enum{
		UPDATE_NOTE_MAG = 0XA0,
		UPDATE_NOTE_SUCCESS = 0XA1,
		UPDATE_NOTE_FAIL = 0XA2
	}UpdateNoteType;

	typedef enum{
		CREATE_SESSION_MAG = 0XB0,
		CREATE_SESSION_SUCCESS = 0XB1,
		CREATE_SESSION_FAIL = 0XB2
	}CreateSessionType;

};

class Handler {
public:   
    //处理请求的方法
    virtual char* handleRequest(cJSON *dataitem) = 0;
};
//连接心跳包信息处理者
class KeepliveMagHandler : public Handler {
public:
	virtual char* handleRequest(cJSON *dataitem);
};

//注册信息处理者
class RegistMagHandler : public Handler {
public:
	virtual char* handleRequest(cJSON *dataitem);
};

//登录信息处理者
class LoginMagHandler : public Handler {
public:
	virtual char* handleRequest(cJSON *dataitem);
};
//发布博客信息处理者
class PublishBlogMagHandler : public Handler {
public:
	virtual char* handleRequest(cJSON *dataitem);
};
//请求此用户所有博客信息处理者
class RequestAllBlogMagHandler : public Handler {
public:
	virtual char* handleRequest(cJSON *dataitem);
};
//关键字搜索博客信息处理者
class KeyWorkFindBlogMagHandler : public Handler {
public:
	virtual char* handleRequest(cJSON *dataitem);
};
//推荐博客信息处理者
class RecommendBlogMagHandler : public Handler {
public:
	virtual char* handleRequest(cJSON *dataitem);
};
//文件信息处理者
class FileMsgHandler : public Handler{
public:
	virtual char* handleRequest(cJSON *dataitem);
};
//资源下载信息处理者
class ResourceDownloadHandler : public Handler{
public:
	virtual char* handleRequest(cJSON *dataitem);
};
//修改笔记信息处理者
class UpdateNoteHandler : public Handler{
public:
	virtual char* handleRequest(cJSON *dataitem);
};
//新建会话
class CreateSessionHandler : public Handler{
public:
	virtual char* handleRequest(cJSON *dataitem);
};

//执行者
class Executor{
public:
	Executor() : handler(nullptr){}
	~Executor(){
		if(handler != nullptr)
		delete handler;
		handler = nullptr;
	}
	void setHandler(Handler *_handler){
		this->handler = _handler;
	}
	char* execWork(cJSON *dataitem){
		return this->handler->handleRequest(dataitem);
	}
private:
	Handler *handler;
};
#endif