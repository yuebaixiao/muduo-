### 知识点梳理 ###
增加了Acceptor类，它包装了accept。它又使用到了SocketOps.h[cc]和InetAddress.cc[h],Socket.cc[h]。

所以先要看下面3个的介绍。
#### InetAddress.cc[h] ####
只有一个成员变量struct sockaddr_in addr_,把设置sockaddr_in的烦躁工作封装起来。让用户只需要提供ip或者端口号就能设定sockaddr_in。

-public 方法：
      * explicit InetAddress(uint16_t port)：用port设定sockaddr_in，addr为0.0.0.0
	  * InetAddress(const std::string& ip, uint16_t port):用ip和port设定sockaddr_in
	  * InetAddress(const struct sockaddr_in& addr):用sockaddr_in设定addr_
	  * toHostPort：返回ip:port格式的string
	  * getSockAddrInet:返回addr_
	  * setSockAddrInet:设置addr_

#### SocketsOps.cc[h] ####
封装了socket(2),bind(2),listen(2),accept(2)和大小端转换的函数。

- hostToNetwork64：主机字节序转换成网络字节序
- hostToNetwork32：主机字节序转换成网络字节序
- hostToNetwork16：主机字节序转换成网络字节序
- networkToHost64：网络字节序转换成主机字节序
- networkToHost32：网络字节序转换成主机字节序
- networkToHost16：网络字节序转换成主机字节序
- createNonblockingOrDie：包装socket(2)系统调用。直接指定SOCK_NONBLOCK和SOCK_CLOEXEC。所以创建的fd就是非阻塞和cloexec的
- bindOrDie：包装bind(2)系统调用
- listenOrDie:包装listen(2)系统调用
- accept：包装accept(2)系统调用。直接指定SOCK_NONBLOCK和SOCK_CLOEXEC。所以返回的connfd就是非阻塞和cloexec的。
- close:包装close(2)系统调用
- toHostPort：返回ip:port格式的buf
- fromHostPort：用参数来的ip和port设定sockaddr_in

#### Socket.h[cc] ####
包装socket(2),bind(2),listen(2),accept(2)的类，实际调用的是SocketsOps里的函数。

了解了上面3个之后，看看Acceptor类的成员变量。

#### Acceptor.h[cc] ####

- 成员变量：
    - EventLoop* loop_:EventLoop对象指针
    - Socket acceptSocket_:是RAII handle，封装了socket文件描述符的声明周期
    - Channel acceptChannel_:用户观察此socket上的readable事件，并回调Acceptor::hanleRead，后者会调用accept(2)来接受新连接，并回调用户的callback
    - NewConnectionCallback newConnectionCallback_:
    - bool listenning_:是否调用了listen接口
- private 方法：
    - handleRead：acceptSocket_变成readable后，调用此方法，此方法会调用accept(2)来接受新连接，并回调用户的callback。此方法必须在IO线程里被调用
	
- public 方法：
    - Acceptor(EventLoop* loop, const InetAddress& listenAddr)：创建acceptSocket_对象，acceptChannel_对象，让此端口可以重用，调用bind(2).设置acceptChannel_的回调函数是Acceptor::handleRead。
	- listen：调用listen(2).此方法必须在IO线程里被调用。把此socket的读事件加入到poller。
