### 知识点梳理 ###

#### 1，Channel ####
- Channell类对象自始至终只属于一个EventLoop，因此每个Channel对象都只属于某一个IO线程。每个Channel对象自始至终只负责一个文件描述符（fd）的事件分发，但并不拥有这个fd，也不会在析构的时候关闭这个fd。
- 用户不需要直接使用Channel，而是使用更上层的封装，比如TcpServer类。
- Channel的生命周期由其owner class负责管理，它一般是其他class的直接或间接成员。
- 成员变量：
    - int events_:关注的事件
    - int revents_:poll或者epoll_wait返回的事件
    - int index_: Poller里有一个std::vector<struct pollfd> pollfds_，此index_标识出，此Channel在pollfds_里的index。
	- EventCallback readCallback_:可读事件触发后，调用此方法
    - EventCallback writeCallback_;可写事件触发后，调用此方法
    - EventCallback errorCallback_;error事件触发后，调用此方法
- 私有成员方法
    - void update():调用EventLoop对象的updateChannel方法。目的是往poller里添加fd_的监听事件（读或者写）
- 共有成员方法
    - 构造方法：初始化loop_,fd_；把events_,revents_设置为0，index_为-1(由于此Channel对象还没有被添加到Poller的pollfds_(vector)，所以为-1)
	- void handleEvent():根据revents_的值，决定执行哪个回调函数
	- void enableReading():设置events_的值，并调用私有函数update。目的是往poller里添加fd_的监听事件（读或者写）
	- int index():返回index_
    - set_index(int idx):设置index_
    - EventLoop* ownerLoop():返回loop_

#### 2，Poller ####
- 核心功能是Poller:poll方法，它调用poll或epoll_wait获得当前活动的IO事件，然后填充到调用测传入的activeChannels_（类型是std::vector<Channel*>），并返回poll或epoll_wait return的时刻
- Poller class是IO multiplexing的封装。虽然它现在是具体的类，而在muduo中是抽象基类，因为muduo同时支持poll2和epoll(7)两种IO multiplexing。Poller是EventLoop的间接成员，只供其owner EventLoop在IO线程调用（EventLoop::loop方法里，调用poll前，会调用assertInLoopThread()来做判断，包装其是在owner EventLoop在IO线程调用里被调用），因此无需加锁。
- 其生命周期与EventLoop相等。

- 成员变量
    - ownerLoop_:所属的EventLoop对象
	- pollfds_:类型是std::vector<struct pollfd>，poll(2)使用的结构体的vector
	- channels_:类型是std::map<int, Channel*> ChannelMap，里面放的是fd和Channel的对应关系。
- public方法
    - Poller(EventLoop* loop)：初始化ownerLoop_
	- poll:核心方法，调用poll(2)后，pollfds_被更新，然后调用私有方法fillActiveChannels，目的是把更新了的pollfds_信息，存入到相应的Channle里，并把此Channle放入传进来的参数activeChannels里。在调用测处理activeChannels，从里面取出每一个Channle，并调用Channle的handleEvent，handleEvent里面回调了用户注册的方法。
	- updateChannel：由EventLoop::updateChannel调用此方法，用于更新pollfds_，更新了pollfds_，就更新了poll(2)监听的fd和fd的事件。存在2个分支。
     - 分支1：要更新的Channel在pollfds_里不存在(Channle对象的index<0)
	   创建一个struct pollfd，把Channel里的值，设置到struct pollfd里，并把此struct pollfd，push_back到pollfds_里，并把此Channle插入到channels_里。
	 - 分支2：要更新的Channel在pollfds_里存在
	   用Channle的index_，从pollfds_取出struct pollfd，并根据Channel里的值更新struct pollfd。
	   如果Channle->isNoneEvent()返回true，则把struct pollfd的fd设置为-1，目的是让poll(2)忽略此fd.

- private方法：
  -	fillActiveChannels 
#### 3，EventLoop ####
