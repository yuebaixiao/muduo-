
#include "EventLoop.h"

#include <boost/bind.hpp>

#include <stdio.h>

muduo::EventLoop* g_loop;

void print()
{
  printf("pid:%d, tid:%d\n", 11 ,22);
  
}

void threadFunc()
{
  g_loop->runInLoop(print);
}

int main()
{
  muduo::EventLoop loop;
  g_loop = &loop;
  loop.loop();
  muduo::Thread t(threadFunc);
  t.start();
  //loop.loop();
  //t.join();
  // sleep(1);
  
  
}
