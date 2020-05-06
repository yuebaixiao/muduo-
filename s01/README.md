### 知识点梳理 ###

#### 1，Channel ####
- Channell类对象自始至终只属于一个EventLoop，因此每个Channel对象都只属于某一个IO线程。每个Channel对象自始至终只负责一个文件描述符（fd）的事件分发，但并不拥有这个fd，也不会在析构的时候关闭这个fd。
- 用户不需要直接使用Channel，而是使用更上层的封装，比如TcpServer类。

- 成员变量：
    - int events_:关注的事件
    - int revents_:poll或者epoll_wait返回的事件
    - int index_: Poller里有一个std::vector<struct pollfd> pollfds_，此index_标识出，此Channel在pollfds_里的index。
	- EventCallback readCallback_:可读事件触发后，调用此方法
    - EventCallback writeCallback_;可写事件触发后，调用此方法
    - EventCallback errorCallback_;error事件触发后，调用此方法
#### 2，Poller ####

#### 3，EventLoop ####
