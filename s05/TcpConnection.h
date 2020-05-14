// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_NET_TCPCONNECTION_H
#define MUDUO_NET_TCPCONNECTION_H

#include "Callbacks.h"
#include "InetAddress.h"

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace muduo
{

class Channel;
class EventLoop;
class Socket;

///
/// TCP connection, for both client and server usage.
///
class TcpConnection : boost::noncopyable,
                      public boost::enable_shared_from_this<TcpConnection> // 继承此类的目的：用户的回调函数的参数是TcpConnection对象，为了保证执行用户回调的时候，此TcpConnection对象还存活着。
{
 public:
  /// Constructs a TcpConnection with a connected sockfd
  ///
  /// User should not create this object.
  TcpConnection(EventLoop* loop,
                const std::string& name, // 连接的名字:ip:port#连接序号
                int sockfd,		 // listen fd
                const InetAddress& localAddr, // listen fd 监听的ip和port
                const InetAddress& peerAddr); // 客户端的ip和port
  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddress& localAddress() { return localAddr_; }
  const InetAddress& peerAddress() { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }

  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

  /// Internal use only.

  // called when TcpServer accepts a new connection
  void connectEstablished();   // should be called only once 当listen fd有新的连接时，TcpServer::newConnection会调用此方法，目的是：1，回调用户的当连接建立后的函数。2，把accept(2)返回的fd，加入到poller里。

 private:
  enum StateE { kConnecting, kConnected, };

  void setState(StateE s) { state_ = s; }
  void handleRead();		// 客户端有数据发送过来后，执行此方法

  EventLoop* loop_;
  std::string name_;
  StateE state_;  // FIXME: use atomic variable
  // we don't expose those classes to client.
  boost::scoped_ptr<Socket> socket_; // 封装客户端fd的socket
  boost::scoped_ptr<Channel> channel_; // 封装客户端fd的channel
  InetAddress localAddr_;	       // listen fd所监听的ip和port
  InetAddress peerAddr_;	       // client fd是由哪个ip和port来连接的
  ConnectionCallback connectionCallback_; // 用户指定的，当连接建立后，执行的callback
  MessageCallback messageCallback_;	  // 用户指定的，当客户端往server写数据后，执行的callback
};

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

}

#endif  // MUDUO_NET_TCPCONNECTION_H
