#include "Message.h"
#include <string.h>
#include "../LMysql/connect_pool.h"
#include "../util/Singleton.h"
#include "../Llib/Logger.h"
char* Message::messageAnalysis(const char* message){
	LogInfo(NULL);
	char *retMag;
	retMag = NULL;
	//解析CJSON
	cJSON *root = cJSON_Parse(message);
	if(root == 0){
		LogInfo(NULL);
		return retMag;
	}
	cJSON *typeitem = cJSON_GetObjectItem(root,"type");
	//printf("type : %d\n",typeitem->valueint);
	cJSON *dataitem = cJSON_GetObjectItem(root,"data");
	Executor *executor = new Executor();
	switch(typeitem->valueint){
		case Message::KEEPLIVE_MAG :
								  executor->setHandler(new KeepliveMagHandler());break;  
		case Message::REGIST_MAG : 
								  executor->setHandler(new RegistMagHandler());break;  
		case Message::LOGIN_MAG :
								  executor->setHandler(new LoginMagHandler());break;	
		case Message::BLOG_MAG  : 
								  executor->setHandler(new PublishBlogMagHandler());break;
		case Message::LOADBLOG_MAG:
								  executor->setHandler(new RequestAllBlogMagHandler());break;
		case Message::FINDBLOG_MAG:
								  executor->setHandler(new KeyWorkFindBlogMagHandler());break;
		case Message::RECOMBLOG_MAG:
								  executor->setHandler(new RecommendBlogMagHandler());break;
		case Message::FILE_MAG:
								  executor->setHandler(new FileMsgHandler());break;
		case Message::FILE_DOWNLOAD_MAG:
								  executor->setHandler(new ResourceDownloadHandler());break;						  
		case Message::UPDATE_NOTE_MAG:
								  executor->setHandler(new UpdateNoteHandler());break;
	}
	retMag = executor->execWork(dataitem);
	delete(executor);
	executor = NULL;
	//释放根节点
	cJSON_Delete(root);
	return retMag;
}

//处理连接心跳包信息
char* KeepliveMagHandler::handleRequest(cJSON *_dataitem){
	LogInfo(NULL);
	return NULL;
}

//处理注册信息
char* RegistMagHandler::handleRequest(cJSON *_dataitem){
	LogInfo(NULL);
	cJSON *usernamecJSON = cJSON_GetObjectItem(_dataitem,"username");
	cJSON *passwordcJSON = cJSON_GetObjectItem(_dataitem,"password");
	char *username = usernamecJSON->valuestring;
	char *password = passwordcJSON->valuestring;
	//printf("username : %s\n password : %s\n",username,password);
	//数据库连接池中的连接索引
	int index = Singleton<connect_pool>::getInstance().get_connect_index();
	int type;
	//读写锁
	pthread_rwlock_t rwlock;
	//初始化锁
	pthread_rwlock_init(&rwlock, NULL);
	string querySql = "select * from bloguser where user_name='";
	querySql += username;
	querySql += "\';";
	//cout << querySql << endl;
	//判断该用户是否存在
	if(Singleton<connect_pool>::getInstance().get_connect(index)->existRecord(querySql)){
		type = Message::REGIST_FAIL;//验证存在，注册失败类型
	}else{
		type = Message::REGIST_SUCCESS;//验证不存在，注册成功类型
		pthread_rwlock_wrlock(&rwlock);//请求写锁
		//插入数据库
		MysqlHelper::RECORD_DATA record;
		record.insert(make_pair("user_name",make_pair(MysqlHelper::DB_STR,username)));
		record.insert(make_pair("user_password",make_pair(MysqlHelper::DB_STR,password)));
		Singleton<connect_pool>::getInstance().get_connect(index)->insertRecord("bloguser",record);;
		pthread_rwlock_unlock(&rwlock);//解锁
	}
	Singleton<connect_pool>::getInstance().remove_connect_from_pool(index);
	//生成JSON
	cJSON *creat_root = cJSON_CreateObject();
	cJSON_AddNumberToObject(creat_root,"type",type);//注册状态类型
	cJSON *dataitem = cJSON_CreateObject();
	cJSON_AddNumberToObject(dataitem,"userid",0);
	cJSON_AddItemToObject(creat_root,"data",dataitem);
	char *retbuf = cJSON_Print(creat_root);
	//删除
	cJSON_Delete(creat_root);
	return retbuf;
}

//处理登录信息
char* LoginMagHandler::handleRequest(cJSON *_dataitem){
	LogInfo(NULL);
	if(_dataitem == NULL) { cout << "_dataitem = null" <<endl;return NULL;}
	cJSON *username = cJSON_GetObjectItem(_dataitem,"username");
	cJSON *password = cJSON_GetObjectItem(_dataitem,"password");
	int type;
	int user_id = -1;
	int index = Singleton<connect_pool>::getInstance().get_connect_index();
	LogInfo(NULL);
	//连接数达到上限
	if(index == -1){
		LogInfo(NULL);return NULL;
	}
	//判断该用户是否存在
	string querySql = "select * from bloguser where user_name='";
	querySql += username->valuestring;
	querySql += "\' and user_password='";
	querySql += password->valuestring;
	querySql += "\'";//去掉;
	LogInfo(NULL);
	MysqlHelper* mysqlcon = Singleton<connect_pool>::getInstance().get_connect(index);
	if(mysqlcon == NULL)
	{
		LogInfo(NULL);
		return NULL;
	}
	LogInfo(NULL);	
	if(mysqlcon->existRecord(querySql)){ //有问题
		LogInfo(NULL);
		type = Message::LOGIN_SUCCESS;//验证成功类型
		//获取用户id
		MysqlHelper::MysqlData dataSet;
		dataSet=mysqlcon->queryRecord(querySql);
		if(dataSet.size() == 0)
		{
			Singleton<connect_pool>::getInstance().remove_connect_from_pool(index);
			LogInfo(NULL);
			return NULL;
		}		
		user_id = atoi(dataSet[0]["user_id"].c_str());
	}
	else
	{
		type = Message::LOGIN_FAIL;//验证失败类型
	}
	LogInfo(NULL);
	//生成JSON
	cJSON *creat_root = cJSON_CreateObject();
	cJSON_AddNumberToObject(creat_root,"type",type);//登录成功类型
	LogInfo(NULL);
	cJSON *dataitem = cJSON_CreateObject();
	cJSON_AddNumberToObject(dataitem,"userid",user_id);//登录成功返回用户ID
	LogInfo(NULL);
	cJSON_AddItemToObject(creat_root,"data",dataitem);
	char *retbuf = cJSON_Print(creat_root);
	LogInfo(NULL);
	//释放数据库对象到连接池
	Singleton<connect_pool>::getInstance().remove_connect_from_pool(index);
	LogInfo(NULL);
	//删除
	cJSON_Delete(creat_root);
	LogInfo(NULL);
	return retbuf;
}
char* PublishBlogMagHandler::handleRequest(cJSON *_dataitem){
	LogInfo(NULL);
	cJSON *userid = cJSON_GetObjectItem(_dataitem,"userid");
	cJSON *blogtext = cJSON_GetObjectItem(_dataitem,"blogtext");
	cJSON *bloghead = cJSON_GetObjectItem(_dataitem,"bloghead");
	cJSON *pubtime = cJSON_GetObjectItem(_dataitem,"pubtime");
	//printf("userid : %d\n blogtext : %s\n bloghead : %s\n pubtime : %s\n",userid->valueint,blogtext->valuestring,bloghead->valuestring,pubtime->valuestring);
	int index = Singleton<connect_pool>::getInstance().get_connect_index();
	int type = Message::BLOG_SUCCESS;//发布文章成功类型;
	int maxBlogEditId = -1;
	//读写锁
	pthread_rwlock_t rwlock;
	//初始化锁
	pthread_rwlock_init(&rwlock, NULL);
	pthread_rwlock_wrlock(&rwlock);//请求写锁
	//插入blog数据库
	MysqlHelper::RECORD_DATA record;
	record.insert(make_pair("user_id",make_pair(MysqlHelper::DB_INT,std::to_string(userid->valueint))));
	record.insert(make_pair("blog_title",make_pair(MysqlHelper::DB_STR,bloghead->valuestring)));
	record.insert(make_pair("blog_content",make_pair(MysqlHelper::DB_STR,blogtext->valuestring)));
	record.insert(make_pair("blog_time",make_pair(MysqlHelper::DB_STR,pubtime->valuestring)));
	Singleton<connect_pool>::getInstance().get_connect(index)->insertRecord("blog",record);
	
	//获取blog_id字段的最大值
	string condition = "where user_id=";
	condition += std::to_string(userid->valueint);
	maxBlogEditId = Singleton<connect_pool>::getInstance().get_connect(index)->getMaxValue("blog","blog_id",condition);
	if(maxBlogEditId == -1){
		type = Message::BLOG_FAIL;//发布文章失败类型
	}
	pthread_rwlock_unlock(&rwlock);//解锁
	//生成JSON
	cJSON *creat_root = cJSON_CreateObject();
	cJSON_AddNumberToObject(creat_root,"type",type);//发布文章成功类型
	cJSON *dataitem = cJSON_CreateObject();
	cJSON_AddNumberToObject(dataitem,"blogEditid",maxBlogEditId);//返回用户的最大文章编号
	cJSON_AddItemToObject(creat_root,"data",dataitem);
	char *retbuf = cJSON_Print(creat_root);
	Singleton<connect_pool>::getInstance().remove_connect_from_pool(index);
	//删除
	cJSON_Delete(creat_root);
	return retbuf;
}

char *UpdateNoteHandler::handleRequest(cJSON *_dataitem){
	LogInfo(NULL);
	cJSON *userid = cJSON_GetObjectItem(_dataitem,"userid");
	cJSON *noteid = cJSON_GetObjectItem(_dataitem,"noteid");
	cJSON *blogtext = cJSON_GetObjectItem(_dataitem,"blogtext");
	cJSON *bloghead = cJSON_GetObjectItem(_dataitem,"bloghead");
	cJSON *pubtime = cJSON_GetObjectItem(_dataitem,"pubtime");
	
	int index = Singleton<connect_pool>::getInstance().get_connect_index();
	int type = Message::UPDATE_NOTE_SUCCESS;//修改成功类型;
	//读写锁
	pthread_rwlock_t rwlock;
	//初始化锁
	pthread_rwlock_init(&rwlock, NULL);
	pthread_rwlock_wrlock(&rwlock);//请求写锁
	//插入blog数据库
	MysqlHelper::RECORD_DATA record;
	
	record.insert(make_pair("blog_title",make_pair(MysqlHelper::DB_STR,bloghead->valuestring)));
	record.insert(make_pair("blog_content",make_pair(MysqlHelper::DB_STR,blogtext->valuestring)));
	record.insert(make_pair("blog_time",make_pair(MysqlHelper::DB_STR,pubtime->valuestring)));
	char* condition = (char*)malloc(sizeof(char)*100);
	memset(condition,'\0',100);
	sprintf(condition,"where user_id=%d and blog_id=%d",userid->valueint,noteid->valueint);
	Singleton<connect_pool>::getInstance().get_connect(index)->updateRecord("blog",record,string(condition));
	
	pthread_rwlock_unlock(&rwlock);//解锁
	Singleton<connect_pool>::getInstance().remove_connect_from_pool(index);
	return NULL;
}

char *RecommendBlogMagHandler::handleRequest(cJSON *_dataitem){
/*
	LogInfo(NULL);
	cJSON *userid = cJSON_GetObjectItem(_dataitem,"userid");
	
	int index = connect_pool::get_instance()->get_connect_index();
	int type;
	//读写锁
	pthread_rwlock_t rwlock;
	//初始化锁
	pthread_rwlock_init(&rwlock, NULL);
	type = Message::RECOMBLOG_SUCCESS;//成功类型
	pthread_rwlock_wrlock(&rwlock);//请求写锁
	//从数据库获取关键字搜索的博客记录
	MysqlHelper::MysqlData dataSet;
	string querySQL="select * from blog where user_id!=";
	querySQL += std::to_string(userid->valueint);
	querySQL += " limit 0,10;";
	dataSet = connect_pool::get_instance()->get_connect(index)->queryRecord(querySQL);
	
	pthread_rwlock_unlock(&rwlock);//解锁
	//解析results生成JSON
	cJSON *creat_root = cJSON_CreateObject();
	cJSON_AddNumberToObject(creat_root,"type",type);//成功类型
	
	cJSON * blogEdits = NULL;
    blogEdits = cJSON_CreateArray();//创建一个数组
	for(int i=0;i<dataSet.size();i++){
		cJSON *dataitem = cJSON_CreateObject();
		cJSON_AddNumberToObject(dataitem,"userid",atoi(dataSet[i]["user_id"].c_str ()));
		cJSON_AddNumberToObject(dataitem,"blogeditid",atoi(dataSet[i]["blog_id"].c_str ()));
		cJSON_AddStringToObject(dataitem,"blogtext",dataSet[i]["blog_content"].c_str ());
		cJSON_AddStringToObject(dataitem,"bloghead",dataSet[i]["blog_title"].c_str ());
		cJSON_AddStringToObject(dataitem,"pubtime",dataSet[i]["blog_time"].c_str ());
		cJSON_AddItemToArray(blogEdits,dataitem);//加到数组中
	}

	cJSON_AddItemToObject(creat_root,"data",blogEdits);
	char *retbuf = cJSON_Print(creat_root);
	connect_pool::get_instance()->remove_connect_from_pool(index);
	//删除
	cJSON_Delete(creat_root);
	return retbuf;*/
}

char *KeyWorkFindBlogMagHandler::handleRequest(cJSON *_dataitem){
/*
	cout << "KeyWorkFindBlogMagHandler::handleRequest()" <<endl;
	cJSON *userid = cJSON_GetObjectItem(_dataitem,"userid");
	cJSON *keywork = cJSON_GetObjectItem(_dataitem,"keyword");

	int index = connect_pool::get_instance()->get_connect_index();
	int type;
	//读写锁
	pthread_rwlock_t rwlock;
	//初始化锁
	pthread_rwlock_init(&rwlock, NULL);
	type = Message::FINDBLOG_SUCCESS;//成功类型
	pthread_rwlock_wrlock(&rwlock);//请求写锁
	//从数据库获取关键字搜索的博客记录
	MysqlHelper::MysqlData dataSet;
	string querySQL="select * from blog where blog_title like '%";
	querySQL += keywork->valuestring;
	querySQL += "%' limit 0,10;";
	dataSet = connect_pool::get_instance()->get_connect(index)->queryRecord(querySQL);
	
	pthread_rwlock_unlock(&rwlock);//解锁
	//解析results生成JSON
	cJSON *creat_root = cJSON_CreateObject();
	cJSON_AddNumberToObject(creat_root,"type",type);//成功类型
	
	cJSON * blogEdits = NULL;
    blogEdits = cJSON_CreateArray();//创建一个数组
	for(int i=0;i<dataSet.size();i++){
		cJSON *dataitem = cJSON_CreateObject();
		cJSON_AddNumberToObject(dataitem,"userid",atoi(dataSet[i]["user_id"].c_str ()));
		cJSON_AddNumberToObject(dataitem,"blogeditid",atoi(dataSet[i]["blog_id"].c_str ()));
		cJSON_AddStringToObject(dataitem,"blogtext",dataSet[i]["blog_content"].c_str ());
		cJSON_AddStringToObject(dataitem,"bloghead",dataSet[i]["blog_title"].c_str ());
		cJSON_AddStringToObject(dataitem,"pubtime",dataSet[i]["blog_time"].c_str ());
		cJSON_AddItemToArray(blogEdits,dataitem);//加到数组中
	}

	cJSON_AddItemToObject(creat_root,"data",blogEdits);
	char *retbuf = cJSON_Print(creat_root);
	connect_pool::get_instance()->remove_connect_from_pool(index);
	//删除
	cJSON_Delete(creat_root);
	return retbuf;*/
}
char *RequestAllBlogMagHandler::handleRequest(cJSON *_dataitem){

	cout << "RequestAllBlogMagHandler::handleRequest()" <<endl;
	cJSON *userid = cJSON_GetObjectItem(_dataitem,"userid");

	int index = Singleton<connect_pool>::getInstance().get_connect_index();
	int type;
	//读写锁
	pthread_rwlock_t rwlock;
	//初始化锁
	pthread_rwlock_init(&rwlock, NULL);
	type = Message::LOADBLOG_SUCCESS;//成功类型
	pthread_rwlock_wrlock(&rwlock);//请求写锁
	//从数据库获取此用户所有博客记录
	MysqlHelper::MysqlData dataSet;
	string querySQL="select * from blog where user_id=";
	querySQL += std::to_string(userid->valueint);
	querySQL += ";";
	dataSet = Singleton<connect_pool>::getInstance().get_connect(index)->queryRecord(querySQL);
	pthread_rwlock_unlock(&rwlock);//解锁
	//解析results生成JSON
	cJSON *creat_root = cJSON_CreateObject();
	cJSON_AddNumberToObject(creat_root,"type",type);//成功类型
	
	cJSON * blogEdits = NULL;
    blogEdits = cJSON_CreateArray();//创建一个数组
	for(int i=0;i<dataSet.size();i++){
		cJSON *dataitem = cJSON_CreateObject();
		cJSON_AddNumberToObject(dataitem,"blogeditid",atoi(dataSet[i]["blog_id"].c_str ()));
		cJSON_AddStringToObject(dataitem,"blogtext",dataSet[i]["blog_content"].c_str ());
		cJSON_AddStringToObject(dataitem,"bloghead",dataSet[i]["blog_title"].c_str ());
		cJSON_AddStringToObject(dataitem,"pubtime",dataSet[i]["blog_time"].c_str ());
		cJSON_AddItemToArray(blogEdits,dataitem);//加到数组中
	}
	cJSON_AddItemToObject(creat_root,"data",blogEdits);
	char *retbuf = cJSON_Print(creat_root);
	Singleton<connect_pool>::getInstance().remove_connect_from_pool(index);
	//删除
	cJSON_Delete(creat_root);
	return retbuf;
}

bool writePicFile(char *filename,int filesize,unsigned char *filetext)
{
	/*FILE* fp;
	cout<<"Enter the name you wanna to save:" << filename <<endl;
	char filePath[100];
	strcpy(filePath,"./resources/");
	strcat(filePath,filename);
	//以二进制写入方式
	if ( (fp=fopen(filePath, "wb"))==NULL)
	{
		cout<<"Open File failed!"<<endl;
		return false;
	}
	//从buffer中写数据到fp指向的文件中
	fwrite(filetext, filesize, 1, fp);
	cout<<"Done!"<<endl;
	//关闭文件指针，释放buffer内存
	fclose(fp);
	return true;*/
}

unsigned char * readPicFile(char *filename,int &filesize){
	/*//文件指针
	FILE* fp;
	//输入要读取的图像名
	cout<<"Enter Image name:";
	char filePath[100];
	strcpy(filePath,"./resources/");
	strcat(filePath,filename);
	//以二进制方式打开图像
	if ( (fp=fopen(filePath, "rb" ))==NULL )
	{
	cout<<"Open image failed!"<<endl;
	exit(0);
	}
	//获取图像数据总长度
	fseek(fp, 0, SEEK_END);
	filesize=ftell(fp);
	rewind(fp);
	//根据图像数据长度分配内存buffer
	unsigned char* ImgBuffer=(unsigned char*)malloc( filesize* sizeof(unsigned char) );
	//将图像数据读入buffer
	fread(ImgBuffer, filesize, 1, fp);
	fclose(fp);
	return ImgBuffer;*/
}
char *ResourceDownloadHandler::handleRequest(cJSON *_dataitem){
	/*
	cout << "ResourceDownloadHandler::handleRequest()" <<endl;
	cJSON *Jsonuserid = cJSON_GetObjectItem(_dataitem,"userid");
	cJSON *Jsonresourceid = cJSON_GetObjectItem(_dataitem,"resourceid");
	
	//保存到数据库
	int index = connect_pool::get_instance()->get_connect_index();
	//获取当前该用户该编号的资源
	string filename;
	string filetext;
	int filesize;
	connect_pool::get_instance()->get_connect(index)->queryDownloadResource(Jsonuserid->valueint,Jsonresourceid->valueint,filetext,filename,filesize);
	int type;
	type = Message::FILE_DOWNLOAD_SUCCESS;//上传资源成功类型
	connect_pool::get_instance()->remove_connect_from_pool(index);
	//生成JSON
	cJSON *creat_root = cJSON_CreateObject();
	cJSON_AddNumberToObject(creat_root,"type",type);//返回类型
	cJSON *dataitem = cJSON_CreateObject();
	cout<<"3"<<endl;
	cJSON_AddStringToObject(dataitem,"filetext",filetext.c_str());//返回下载的资源
	cout<<"filetext = "<<filetext<<endl;
	cJSON_AddStringToObject(dataitem,"filename",filename.c_str());
	cout<<"filename"<<filename<<endl;
	cJSON_AddNumberToObject(dataitem,"filesize",filesize);
	cout << "filesize = " << filesize<<endl;
	cJSON_AddItemToObject(creat_root,"data",dataitem);
	char *retbuf = cJSON_Print(creat_root);
	//删除
	cJSON_Delete(creat_root);
	return retbuf;*/
}
char *FileMsgHandler::handleRequest(cJSON *_dataitem){
	/*
	cout << "FileMsgHandler::handleRequest()" <<endl;
	cJSON *Jsonuserid = cJSON_GetObjectItem(_dataitem,"userid");
	cJSON *Jsonfilename = cJSON_GetObjectItem(_dataitem,"filename");
	cJSON *Jsonfilesize = cJSON_GetObjectItem(_dataitem,"filesize");
	cJSON *Jsonfiletext = cJSON_GetObjectItem(_dataitem,"filetext");
	cJSON *Jsonuploadtime = cJSON_GetObjectItem(_dataitem,"uploadtime");
	cJSON *Jsonfiletype = cJSON_GetObjectItem(_dataitem,"filetype");
	const char *filename = Jsonfilename->valuestring;
	unsigned long filesize = static_cast<unsigned long>(Jsonfilesize->valueint);
	const char *filetext = Jsonfiletext->valuestring;
	const char *uploadtime = Jsonuploadtime->valuestring;
	int filetype = Jsonfiletype->valueint;
	
	//保存到数据库
	int index = connect_pool::get_instance()->get_connect_index();
	//获取当前该用户最大资源编号
	int maxResourceId = connect_pool::get_instance()->get_connect(index)->queryFieldMaxValue(Jsonuserid->valueint,"resources");
	int type;
	//读写锁
	pthread_rwlock_t rwlock;
	//初始化锁
	pthread_rwlock_init(&rwlock, NULL);
	type = Message::FILE_SUCCESS;//上传资源成功类型
	pthread_rwlock_wrlock(&rwlock);//请求写锁
	//插入resources数据库
	bool ret = connect_pool::get_instance()->get_connect(index)->insertToResources(Jsonuserid->valueint,maxResourceId+1,filename,filesize,uploadtime,filetext,filetype);
	if(!ret){
		type = Message::FILE_FAIL;//发布文章失败类型
	}
	pthread_rwlock_unlock(&rwlock);//解锁
	connect_pool::get_instance()->remove_connect_from_pool(index);
	//生成JSON
	cJSON *creat_root = cJSON_CreateObject();
	cJSON_AddNumberToObject(creat_root,"type",type);//返回类型
	cJSON *dataitem = cJSON_CreateObject();
	cJSON_AddNumberToObject(dataitem,"resourceid",maxResourceId+1);//返回用户的最大文章编号
	cJSON_AddItemToObject(creat_root,"data",dataitem);
	char *retbuf = cJSON_Print(creat_root);
	//删除
	cJSON_Delete(creat_root);
	return retbuf;
	*/
	//const string fromstring = filetext;
	//cout<<"fromstring = " << fromstring.size()<<endl;
	//unsigned char *pOut = new unsigned char[Jsonfilesize->valueint];
	//解码 
	//CBase64::Decode(fromstring,pOut,&filesize);
	//int typeflag = writePicFile(filename,filesize,pOut);
}
