#ifndef _MYHTTP_CONN_H
#define _MYHTTP_CONN_H
#include<iostream>
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<sys/sendfile.h>
#include<sys/epoll.h>
#include<sys/fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<stdarg.h>

using namespace std;

class http_conn{
public:
	/*文件名的最大长度*/
	static const int FILENAME_LEN = 200;
	/*读缓冲区的大小*/
	static const int READ_BUFFER_SIZE = 2048;
	/*写缓冲区的大小*/
	static const int WRITE_BUFFER_SIZE = 1024;
	/*HTTP请求方法，现在仅支持GET*/
	enum HTTP_METHOD{ GET = 0,POST,HEAD,PUT,DELETE,TRACE,OPTIONS,CONNECT,PATCH};
	/*解析客户请求时，住状态机所处的状态*/
	enum HTTP_CHECK_STATE{ CHECK_STATE_REQUESTLINE = 0,CHECK_STATE_HEADER,CHECK_STATE_CONTENT};
	/*服务器处理http请求的返回值*/
	enum HTTP_RETURN_CODE{NO_REQUEST,GET_REQUEST,BAD_REQUEST,NO_RESOURCE,FORBIDDEN_REQUEST,FILE_REQUEST,INTERNAL_ERROR,CLOSED_CONNECTION};
	/*行的读取状态*/
	enum LINE_STATUS{LINE_OK = 0,LINE_BAD,LINE_OPEN};
public:
	http_conn(){}
	~http_conn(){}
public:
	/*初始化新接受的连接*/
	void init(int epfd,int sockfd);
	/*关闭连接*/
	void close_conn(bool real_close = true);
	/*处理客户请求*/
	void process();
	/*非阻塞读操作*/
	bool read();
	/*非阻塞写操作*/
	bool write();
private:
	/*初始化连接*/
	void init();
	/*解析HTTP请求*/
	HTTP_RETURN_CODE process_read();
	/*填充HTTP应答*/
	bool process_write(HTTP_RETURN_CODE ret);
	
	/*下面全部用于process_read()分析HTTP请求的子函数*/
	HTTP_RETURN_CODE parse_request_line(char* text);  //解析请求行
	HTTP_RETURN_CODE parse_headers(char* text);	//解析头部字段
	HTTP_RETURN_CODE parse_content(char* text);	//解析消息体
	HTTP_RETURN_CODE do_request();
	char* get_line() { return m_read_buf + m_start_line;} // 获取一行
	LINE_STATUS parse_line(); // 分析一行
	
	/*下面全部用于process_write()填充HTTP应答的子函数*/
	void unmap();
	bool add_response(const char* format,...);
	bool add_content(const char* content);
	bool add_status_line(int status,const char* title);
	bool add_headers(int content_length);
	bool add_content_length(int content_length);
	bool add_linger();
	bool add_blank_line();
	
public:
	// remove /*全局epollfd，用于维护同一个epoll内核事件表*/
	int m_epollfd;
	/*统计用户的数量*/
	static int m_user_count;
private:
	/*该HTTP连接的socket和对方的socket地址*/
	int m_sockfd;
	sockaddr_in m_address;
	
	/*读缓冲区*/
	char m_read_buf[ READ_BUFFER_SIZE ];
	/*标识读缓冲区中已经读入的客户端数据的最后一个字节的下一个位置*/
	int m_read_idx;
	/*当前正在分析的字符在读缓冲区中的位置*/
	int m_checked_idx;
	/*当前正在解析的行的起始位置*/
	int m_start_line;
	/*写缓冲区*/
	char m_write_buf[ WRITE_BUFFER_SIZE ];
	int m_write_idx;
	
	HTTP_CHECK_STATE m_check_state;
	HTTP_METHOD m_method;
	
	/*客户请求的目标文件的完整文件路径，内容等于 根目录 + 文件名*/
	char m_real_file[ FILENAME_LEN];
	/*客户请求的目标文件的文件名*/
	char* m_url;
	/*HTTP的版本号*/
	char* m_version;
	/*主机名*/
	char* m_host;
	int m_content_length;
	/*HTTP请求是否要求保持连接*/
	bool m_linger;
	
	/*客户请求的目标文件被mmap映射到内存中的起始位置*/
	char* m_file_address;
	/*目标文件的状态，是否存在、根目录、可读、文件大小等*/
	struct stat m_file_stat;
	
	
	struct iovec m_iv[2];
	int m_iv_count;
};

#endif