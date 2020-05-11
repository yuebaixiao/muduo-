
#include "EventLoop.h"

#include <boost/bind.hpp>

#include <stdio.h>

muduo::EventLoop* g_loop;

void print()
{
}

void threadFunc()
{
  g_loop->runAfter(1.0, print);
}

int main()
{
  muduo::EventLoop loop;
  g_loop = &loop;
  muduo::Thread t(threadFunc);
  t.start();
  loop.loop();
}
