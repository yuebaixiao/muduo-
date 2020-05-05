## 知识点梳理 ##
1，
### EventLoop.cc[h]的作用 ###
创建一个循环，在循环里执行epoll_wait或者poll系统调用。

### EventLoop.cc[h]的限制 ###
1，在一个线程里只能有一个EventLoop对象
2，创建EventLoop对象的线程和调用此对象的start方法的线程必须是同一个线程
