# LightOi高性能高并发服务器程序框架

架构介绍：
基于Linux平台的C/S架构的模块化组合的轻量级服务器；
负责并发模式切换、事件处理模式、I/O模型、线程池、数据库连接池、日志等设计与实现；
采用了高效的半同步/半异步模式、主从Reactor事件处理模式、协程调度切换处理；
分离了用户任务接口类，并封装了TCP服务器接口类，屏蔽了各个模块内部实现等；

开发环境：ubuntu20.04

编译工具：cmake

编译流程：
# mysql使用ENABLE_MYSQL宏控制
# 不使用mysql编译如下：
mkdir build
cd build
cmake ..
make

# 使用mysql，启动-DENABLE_MYSQL=ON ， 前提ubuntu需要安装mysql
mkdir build
cd build 
cmake .. -DENABLE_MYSQL=ON
make
