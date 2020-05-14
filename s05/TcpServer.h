// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_NET_TCPSERVER_H
#define MUDUO_NET_TCPSERVER_H

#include "Callbacks.h"
#include "TcpConnection.h"

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace muduo
{

class Acceptor;
class EventLoop;

class TcpServer : boost::noncopyable
{
 public:

  TcpServer(EventLoop* loop, const InetAddress& listenAddr);
  ~TcpServer();  // force out-line dtor, for scoped_ptr members.

  /// Starts the server if it's not listenning.
  ///
  /// It's harmless to call it multiple times.
  /// Thread safe.
  void start();

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

 private:
  /// Not thread safe, but in loop
  void newConnection(int sockfd, const InetAddress& peerAddr); // 有新的连接进来时，执行此方法

  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap; // key是listen fd监听的ip:port#连接id

  EventLoop* loop_;  // the acceptor loop
  const std::string name_;		 // 格式是【ip:port#连接id】，ip和port是listen fd监听的ip和port
  boost::scoped_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
  ConnectionCallback connectionCallback_; // 用户提供的callback
  MessageCallback messageCallback_;	  // 用户提供的callback
  bool started_;
  int nextConnId_;  // always in loop thread 第一个客户端的nextConnId_就是1，第二个客户端的nextConnId_就是2.
  ConnectionMap connections_;	// listen fd监听到的所有客户端fd都存放在这里。
};

}

#endif  // MUDUO_NET_TCPSERVER_H
