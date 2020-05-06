#include "EventLoop.h"
#include "thread/Thread.h"
#include <stdio.h>


int main()
{
  printf("main(): pid = %d, tid = %d\n",
         getpid(), muduo::CurrentThread::tid());

  muduo::EventLoop loop;


  loop.loop();
  // 无法在同一个线程里，在第一个EventLoop对象还没有析构的情况下，创建别的EventLoop对象
  muduo::EventLoop loop1;
  
  pthread_exit(NULL);
}
