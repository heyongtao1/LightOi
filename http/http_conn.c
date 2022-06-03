#include "http_conn.h"
/*定义HTTP响应的状态信息*/
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";	//你的请求有语法错误或是本质上不可能满足
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permisson to get file from this server.\n";	//没有访问权限
const char* error_404_tilte = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";	//文件不存在
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n"; // 请求时出现异常

/*根目录*/
const char* doc_root = "/mnt/hgfs/share-2/network/webServer";

int setnonblocking(int fd)
{
	int old_opt = fcntl(fd,F_GETFL);
	int new_opt = old_opt | O_NONBLOCK;
	fcntl(fd,F_SETFL,new_opt);
	return old_opt;
}

void addfd(int epollfd,int fd, bool one_shot)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	if(one_shot){
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
	setnonblocking(fd);
}

void removefd(int epollfd,int fd)
{
	epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,0);
	close(fd);
}

void modfd(int epollfd,int fd,int ev)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLONESHOT | EPOLLET | EPOLLRDHUP | ev;
	epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
}

int http_conn::m_user_count = 0;
//int http_conn::m_epollfd = -1;

/*关闭客户端链接*/
void http_conn::close_conn(bool real_close)
{
    if(real_close && (m_sockfd != -1)){
		removefd(m_epollfd,m_sockfd);
		m_sockfd = -1;
		m_user_count--;
	}
}

void http_conn::init(int epfd,int sockfd)
{
	m_epollfd = epfd;
	m_sockfd = sockfd;
	//m_address = addr;
	addfd(m_epollfd,sockfd,true);
	m_user_count++;
	init();
}
/*处理请求完之后重置信息*/
void http_conn::init()
{
	m_check_state = CHECK_STATE_REQUESTLINE;
	m_linger = false;
	m_method = GET;
	m_url = 0;
	m_version = 0;
	m_content_length = 0;
	m_host = 0;
	m_start_line = 0;
	m_checked_idx = 0;
	m_read_idx = 0;
	m_write_idx = 0;
	memset(m_read_buf,'\0',READ_BUFFER_SIZE);
	memset(m_write_buf,'\0',WRITE_BUFFER_SIZE);
	memset(m_real_file,'\0',FILENAME_LEN);
}
/*解析行*/
http_conn::LINE_STATUS http_conn::parse_line()
{
	char temp;
	/*每一行都以\r\n结束*/
	for(;m_checked_idx < m_read_idx;m_checked_idx++)
	{
		temp = m_read_buf[ m_checked_idx ];
		if(temp == '\r')
		{
			//表示\r后面没有数据了，说明该行不完整，等待下次在读取
			if((m_checked_idx+1) == m_read_idx)
			{
				return LINE_OPEN;
			}
			else if(m_read_buf[m_checked_idx+1] =='\n')
			{
				//\r后面是\n，说明该行完整
				m_read_buf[m_checked_idx++] = '\0';
				m_read_buf[m_checked_idx++] = '\0';
				return LINE_OK;
			}
		}
		else if( temp == '\n')
		{
			if((m_checked_idx >1) && (m_read_buf[m_checked_idx-1] == '\t'))
			{
				//\n前面是\r，说明该行完整
				m_read_buf[m_checked_idx++] = '\0';
				m_read_buf[m_checked_idx++] = '\0';
				return LINE_OK;
			}
			return LINE_BAD;
		}
	}
}

/*解析HTTP请求行，获取请求方法、目标文件名URL、HTTP版本号*/
http_conn::HTTP_RETURN_CODE http_conn::parse_request_line(char* text)
{
	/*  GET http://localhost/sum.html HTTP/1.1  */
	/*依次检验字符串 str1 中的字符，当被检验字符在字符串 str2 中也包含时，则停止检验，并返回该字符位置*/
	m_url = strpbrk(text," \t");
	if(!m_url)
	{
		/*请求行没有空格，说明请求格式错误*/
		return BAD_REQUEST;
	}
	*m_url++ = '\0';
	
	char* method = text;
	/*判断字符串是否相等，忽略大小写*/
	if( strcasecmp(method,"GET") == 0)
	{
		m_method = GET;
	}
	else
	{
		return BAD_REQUEST;
	}
	
	m_url += strspn( m_url," \t");
	m_version = strpbrk( m_url," \t");
	if( ! m_version)
	{
		return BAD_REQUEST;
	}
	*m_version++ = '\0';
	m_version += strspn( m_version," \t");
	if( strcasecmp(m_version,"HTTP/1.1") != 0)
	{
		return BAD_REQUEST;
	}
	if( strncasecmp(m_url,"http://",7) == 0)
	{
		m_url += 7;
		m_url = strchr(m_url,'/');  //获取文件名 ： /filename
	}
	if(!m_url || m_url[0] != '/')
	{
		return BAD_REQUEST;
	}
	m_check_state = CHECK_STATE_HEADER;
	return NO_REQUEST;
}

http_conn::HTTP_RETURN_CODE http_conn::parse_headers(char* text)
{
	/* Connected: Keep-alive 
		
	*/
	/* 遇见空行表示头部字段结束 */
	if(text[0] == '\0')
	{
		/* 如果请求有消息体，则需要读取消息体，状态机转变为	CHECK_STATE_CONTENT*/
		if(m_content_length != 0)
		{
			m_check_state = CHECK_STATE_CONTENT;
			return NO_REQUEST;
		}
		return GET_REQUEST;
	}
	/* 处理头部各个字段，这里处理Connection字段*/
	else if(strncasecmp(text,"Connection:",11) == 0)
	{
		text += 11;
		text += strspn(text," \t");
		if( strcasecmp(text,"keep-alive") == 0)
		{
			m_linger = true;
		}
	}
	/* 处理Content-Length字段*/
	else if(strncasecmp(text,"Content-Length:",15) == 0)
	{
		text += 15;
		text += strspn(text," \t");
		m_content_length = atol(text);
	}
	/* 处理Host字段*/
	else if(strncasecmp(text,"Host:",5) == 0)
	{
		text += 5;
		text += strspn(text," \t");
		m_host = text;
	}
	else{
		printf("oop! unknow header %s\n",text);
	}
	return NO_REQUEST;
}
/* 没有处理消息体，跳过*/
http_conn::HTTP_RETURN_CODE http_conn::parse_content(char* text)
{
	if(m_read_idx >= (m_content_length + m_checked_idx))
	{
		text[ m_content_length ] = '\0';
		return GET_REQUEST;
	}
	return NO_REQUEST;
}

/* 主状态机，分析HTTP请求入口*/
http_conn::HTTP_RETURN_CODE http_conn::process_read()
{
	LINE_STATUS line_status = LINE_OK;
	HTTP_RETURN_CODE ret = NO_REQUEST;
	char* text = 0;
	
	while( ((m_check_state == CHECK_STATE_CONTENT) && ( line_status == LINE_OK) ) || (line_status = parse_line()) == LINE_OK)
	{
		text = get_line();
		m_start_line = m_checked_idx;
		printf("got 1 http line: %s\n",text);
		
		switch(m_check_state)
		{
			//解析请求行
			case CHECK_STATE_REQUESTLINE : 
			{
				ret = parse_request_line(text);
				if(ret == BAD_REQUEST)
				{
					return BAD_REQUEST;
				}
				break;
			}
			//解析头部字段
			case CHECK_STATE_HEADER:
			{
				ret = parse_headers(text);
				if(ret == BAD_REQUEST)
				{
					return BAD_REQUEST;
				}
				else if(ret == GET_REQUEST)
				{
					return do_request();
				}
				break;
			}
			//解析消息体
			case CHECK_STATE_CONTENT:
			{
				ret = parse_content(text);
				if(ret == GET_REQUEST)
				{
					return do_request();
				}
				line_status == LINE_OPEN;
				break;
			}
			default:
			{
				return INTERNAL_ERROR;
			}
		}
	}
	return NO_REQUEST;
}
/* 读取到完整HTTP请求后，分析目标文件的属性，是否存在、访问权限、是否目录等，使用mmap映射到内存地址m_file_address中*/
http_conn::HTTP_RETURN_CODE http_conn::do_request()
{
	strcpy(m_real_file,doc_root);
	int len = strlen(doc_root);
	strncpy(m_real_file + len , m_url,FILENAME_LEN - len -1); // 拼接目标文件路径
	if( stat(m_real_file , &m_file_stat ) < 0)
	{
		return NO_RESOURCE;
	}
	if( !(m_file_stat.st_mode & S_IROTH))
	{
		return FORBIDDEN_REQUEST;
	}
	if( S_ISDIR(m_file_stat.st_mode))
	{
		return BAD_REQUEST;
	}
	/*文件存在，打开目标文件*/
	int fd = open(m_real_file,O_RDONLY);
	/*将文件内容映射到m_file_address内存中*/
	m_file_address = (char*) mmap (0,m_file_stat.st_size,PROT_READ,MAP_PRIVATE,fd,0);
	close(fd);
	return FILE_REQUEST;
}

/* 对内存映射区执行munmap操作,释放内存*/
void http_conn::unmap()
{
	if(m_file_address )
	{
		munmap(m_file_address,m_file_stat.st_size);
		m_file_address = 0;
	}
}

/* 读HTTP请求 保存到读缓冲区中*/
bool http_conn::read()
{
	if(m_read_idx >= READ_BUFFER_SIZE)
	{
		return false;
	}
	int bytes_read = 0;
	while(1)
	{
		bytes_read = recv(m_sockfd,m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx,0);
		if(bytes_read == -1)
		{
			/*读取结束*/
			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				break;
			}
			return false;
		}
		else if(bytes_read == 0)
		{
			return false;
		}
		m_read_idx += bytes_read;
	}
	return true;
}

/* 往写缓冲区中写入待发送的数据*/
bool http_conn::add_response( const char* format,...)
{
	if(m_write_idx >= WRITE_BUFFER_SIZE)
	{
		return false;
	}
	va_list arg_list;
	va_start( arg_list ,format);
	int len = vsnprintf(m_write_buf+m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list);
	if(len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx))
	{
		return false;
	}
	m_write_idx += len;
	va_end(arg_list);
	return true;
}

bool http_conn::add_status_line( int status, const char* title)
{
	return add_response( "%s %d %s\r\n","HTTP/1.1",status,title);
}

bool http_conn::add_headers(int content_len)
{
	add_content_length( content_len);
	add_linger();
	add_blank_line();
}

bool http_conn::add_content_length(int content_len)
{
	return add_response("Content-Length: %d\r\n",content_len);
}

bool http_conn::add_linger()
{
	return add_response("Connection: %s\r\n",m_linger ? "keep-alive" : "clost");
}

bool http_conn::add_blank_line()
{
	return add_response("%s","\r\n");
}

bool http_conn::add_content(const char* content)
{
	return add_response("%s",content);
}

/* 写HTTP响应 */
bool http_conn::write()
{
	int temp = 0;
	int bytes_have_send = 0;
	int bytes_to_send = m_write_idx;
	if( bytes_to_send == 0)
	{
		modfd(m_epollfd,m_sockfd,EPOLLIN);
		init();
		return true;
	}
	while(1)
	{
		temp = writev(m_sockfd,m_iv,m_iv_count);
		if(temp <= -1)
		{
			if(errno == EAGAIN)
			{
				modfd(m_epollfd,m_sockfd,EPOLLOUT);
				return false;
			}
			unmap();
			return false;
		}
		bytes_to_send -= temp;
		bytes_have_send += temp;
		if(bytes_to_send <= bytes_have_send)
		{
			/* 发送HTTP响应成功，释放映射内存，根据字段是否决定关闭连接*/
			unmap();
			if(m_linger)
			{
				init();
				modfd(m_epollfd,m_sockfd,EPOLLIN);
				return true;
			}
			else{
				modfd(m_epollfd,m_sockfd,EPOLLIN);
				return false;
			}
		}
	}
}
/* 根据服务器处理的HTTP请求返回的结果，决定响应客户端的内容*/
bool http_conn::process_write(HTTP_RETURN_CODE ret)
{
	switch(ret)
	{
		case INTERNAL_ERROR:
		{
			add_status_line( 500 , error_500_title);
			add_headers(strlen( error_500_form));
			if( !add_content( error_500_form))
			{
				return false;
			}
			break;
		}
		case BAD_REQUEST:
		{
			add_status_line( 400 ,error_400_title);
			add_headers(strlen(error_400_form));
			if( !add_content( error_400_form))
			{
				return false;
			}
			break;
		}
		case NO_RESOURCE:
		{
			add_status_line( 404 ,error_404_tilte);
			add_headers(strlen(error_404_form));
			if( !add_content( error_404_form))
			{
				return false;
			}
			break;
		}
		case FORBIDDEN_REQUEST:
		{
			add_status_line( 403 ,error_403_title);
			add_headers(strlen(error_403_form));
			if( !add_content( error_403_form))
			{
				return false;
			}
			break;
		}
		case FILE_REQUEST:
		{
			add_status_line( 200 ,ok_200_title);
			if( m_file_stat.st_size != 0)
			{
				add_headers( m_file_stat.st_size);
				m_iv[0].iov_base = m_write_buf;
				m_iv[0].iov_len = m_write_idx;
				m_iv[1].iov_base = m_file_address;
				m_iv[1].iov_len = m_file_stat.st_size;
				break;
			}
			else
			{
				const char* ok_string = "<html><body></body></html>";
				add_headers(strlen(ok_string));
				if( !add_content(ok_string))
				{
					return false;
				}
			}
		}
		default:
		{
			return false;
		}
	}
	m_iv[0].iov_base = m_write_buf;
	m_iv[0].iov_len = m_write_idx;
	m_iv_count = 1;
	return true;
}

/* 由线程池中的工作线程调用，这是处理HTTP请求的入口函数*/
void http_conn::process()
{
	HTTP_RETURN_CODE read_ret = process_read();
	if(read_ret == NO_REQUEST)
	{
		modfd(m_epollfd,m_sockfd,EPOLLIN);
		return;
	}
	bool write_ret = process_write(read_ret);
	if(!write_ret)
	{
		cout << "fd close" << endl;
		close_conn();
	}
	/* 通知主程序该sockfd可写，触发写事件，加入到工作队列中*/
	modfd(m_epollfd,m_sockfd,EPOLLOUT);
}


