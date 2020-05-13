// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "Acceptor.h"

#include "logging/Logging.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

using namespace muduo;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
  : loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDie()), // 调用socket(2)
    acceptChannel_(loop, acceptSocket_.fd()),
    listenning_(false)
{
  acceptSocket_.setReuseAddr(true); // 让端口可以重用
  acceptSocket_.bindAddress(listenAddr); // 调用bind(2)
  acceptChannel_.setReadCallback(
      boost::bind(&Acceptor::handleRead, this)); // 此socket可读后，调用Acceptor::handleRead
}

void Acceptor::listen()
{
  loop_->assertInLoopThread();	// 调用此方法的线程必须是IO线程
  listenning_ = true;
  acceptSocket_.listen();	// 调用listen(2)
  acceptChannel_.enableReading(); // 把此socket的可读事件，加入到poller
}

void Acceptor::handleRead()
{
  loop_->assertInLoopThread();	// 调用此方法的线程必须是IO线程
  InetAddress peerAddr(0);
  //FIXME loop until no more
  int connfd = acceptSocket_.accept(&peerAddr); // 调用accept(2),peerAddr是accept返回的sockaddr_in
  if (connfd >= 0) {
    if (newConnectionCallback_) {
      newConnectionCallback_(connfd, peerAddr); // 执行用户的回调
    } else {
      sockets::close(connfd);	// 如果用户没有设置newConnectionCallback_，则主动断开连接
    }
  }
}

