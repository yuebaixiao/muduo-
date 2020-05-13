#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include <stdio.h>


void func1(int sockfd, const muduo::InetAddress& peerAddr)
{
  printf("newConnection(): accepted a new connection from %s\n",
         peerAddr.toHostPort().c_str());
  ::write(sockfd, "How are you?\n", 13);
  muduo::sockets::close(sockfd);
}

int main()
{
  printf("main: pid=%d\n", getpid());
  muduo::EventLoop loop;

  muduo::InetAddress listenAddr(9980);
  muduo::Acceptor acc1(&loop, listenAddr);
  acc1.setNewConnectionCallback(func1);
  acc1.listen();

  muduo::InetAddress listenAddr1(9981);
  muduo::Acceptor acc2(&loop, listenAddr1);
  acc2.setNewConnectionCallback(func1);
  acc2.listen();

  loop.loop();
  
}
