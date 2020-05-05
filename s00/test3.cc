#include "EventLoop.h"
#include "thread/Thread.h"
#include <stdio.h>


int main()
{
  printf("main(): pid = %d, tid = %d\n",
         getpid(), muduo::CurrentThread::tid());

  muduo::EventLoop loop;


  loop.loop();
  
  muduo::EventLoop loop1;
  
  pthread_exit(NULL);
}
