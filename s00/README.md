## 知识点梳理 ##
1，__thread关键字，每个线程都有独立与别的线程的值。
2，如何判断，初始化对象的线程，和调用此对象方法的线程是否是同一个线程。
3，__builtin_expect是gcc编程器里的优化指令。
cpu在执行当前指令时，从内存中取出了当前指令的下一条指令。执行完当前指令后，cpu发现不是要执行下一条指令,而是执行offset偏移处的指令。cpu只能重新从内存中取出offset偏移处的指令。因此，跳转指令会降低流水线的效率，也就是降低cpu的效率。
在写程序时应该尽量避免跳转语句。那么如何避免跳转语句呢？答案就是使用__builtin_expect。

## EventLoop.cc[h]的作用 ##

创建一个循环，在循环里执行epoll_wait或者poll系统调用。

### EventLoop.cc[h]的限制 ###
1，在一个线程里只能有一个EventLoop对象
- 实现原理：
使用全局变量：__thread EventLoop* t_loopInThisThread = 0;
在构造方法里，t_loopInThisThread的初始值是0，所以被更新成this，在同一个线程里再次创建EventLoop对象时，由于t_loopInThisThread不为0，就进入了if分支，程序down掉，就实现了在一个线程里只能有一个EventLoop对象。
```c++
  if (t_loopInThisThread)
  {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  }
  else
  {
    t_loopInThisThread = this;
  }
```

2，创建EventLoop对象的线程和调用此对象的loop方法的线程必须是同一个线程
- 实现原理：
loop方法里，首先调用assertInLoopThread方法，来确认是否处于同一个线程。
EventLoop类的threadId_成员变量，在构造方法里被初始化为当前的线程的线程ID。然后在assertInLoopThread方法里，判断调用者的线程ID和threadId_是否相等，如果相等则说明是在同一个线程。
注意：linux里的线程实际是用进程来实现的，线程id没有实际作用，所以就用进程id，代替了线程id，使用系统调用syscall(SYS_gettid)得到线程的进程id。
CurrentThread::tid()方法里使用了if (__builtin_expect(t_cachedTid == 0, 0))，__builtin_expect的作用是：https://blog.csdn.net/shuimuniao/article/details/8017971


- asdf
 - asdfaaa 
  - ds
