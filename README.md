# LightOi
基于Linux的多线程Reactor + one thread per  loop组合模式的轻量级服务器
采用了高效的半同步/半异步模式、Reactor 事件处理模式、epoll 的 I/O 复用等
整体使用了模块化组合，分为四大模块：
1、Rector模块（MainReactor、SubReactor均继承Reactor基类）
MainReactor类：称为主Reactor，只负责监听新客户连接事件。
SubReactor类 ：称为子Reactor，只负责监听已连接的活动事件。
2、Acceptor模块 ： 接收器（只负责接受新连接，使用回调方式把新连接分配到子Reactor，可以采用Round Robin算法实现类似负载均衡）
3、TcpServer模块 ： 其他模块的组合，设置地址和端口即可开启服务器。
4、User模块 ： 该模块为用户自定义设置，项目中采用的http web模块测试
