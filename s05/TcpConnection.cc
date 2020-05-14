// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "TcpConnection.h"

#include "logging/Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

#include <boost/bind.hpp>

#include <errno.h>
#include <stdio.h>

using namespace muduo;

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
  : loop_(CHECK_NOTNULL(loop)),
    name_(nameArg),
    state_(kConnecting),
    socket_(new Socket(sockfd)), // 客户端socket
    channel_(new Channel(loop, sockfd)), // 客户端channel
    localAddr_(localAddr),		 // server的ip和port
    peerAddr_(peerAddr)			 // 客户端的ip和prot
{
  LOG_DEBUG << "TcpConnection::ctor[" <<  name_ << "] at " << this
            << " fd=" << sockfd;
  channel_->setReadCallback(
      boost::bind(&TcpConnection::handleRead, this)); // 当客户端fd可读时，调用handleRead
}

TcpConnection::~TcpConnection()
{
  LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
            << " fd=" << channel_->fd();
}

void TcpConnection::connectEstablished()
{
  loop_->assertInLoopThread();	// 必须在IO线程执行此函数
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->enableReading();	// 把客户端fd加入到poller里

  connectionCallback_(shared_from_this()); // 执行callback，必须使用shared_from_this，否则无法保证在用户的回调函数里，此TcpConnection对象是否还存活着。
}

void TcpConnection::handleRead()
{
  char buf[65536];
  ssize_t n = ::read(channel_->fd(), buf, sizeof buf); // 从客户端fd读数据
  messageCallback_(shared_from_this(), buf, n);	       // 调用用户的callback
  // FIXME: close connection if n == 0
}

