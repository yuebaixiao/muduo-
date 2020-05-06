// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace muduo
{

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd
class Channel : boost::noncopyable
{
 public:
  typedef boost::function<void()> EventCallback;

  Channel(EventLoop* loop, int fd);
  // 设置events_的值，并调用私有函数update。目的是往poller里添加fd_的监听事件（读或者写）
  void handleEvent();
  void setReadCallback(const EventCallback& cb)
  { readCallback_ = cb; }
  void setWriteCallback(const EventCallback& cb)
  { writeCallback_ = cb; }
  void setErrorCallback(const EventCallback& cb)
  { errorCallback_ = cb; }

  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }

  void enableReading() { events_ |= kReadEvent; update(); }
  // void enableWriting() { events_ |= kWriteEvent; update(); }
  // void disableWriting() { events_ &= ~kWriteEvent; update(); }
  // void disableAll() { events_ = kNoneEvent; update(); }

  // for Poller
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  EventLoop* ownerLoop() { return loop_; }

 private:
  void update();

  static const int kNoneEvent;	// 0
  static const int kReadEvent;	// POLLIN | POLLPRI（读）
  static const int kWriteEvent;	// POLLOUT（写）

  EventLoop* loop_;		// Channel对象所属的EventLoop对象
  const int  fd_;		// Channel负责的fd
  int        events_;		// Channel关注的事件
  int        revents_;		// poll或epoll_wait返回后，fd的事件
  int        index_;		// Poller里有一个std::vector<struct pollfd> pollfds_，此index_标识出，此Channel在pollfds_里的index。

  EventCallback readCallback_;	// 可读事件触发后，调用此方法
  EventCallback writeCallback_;	// 可写事件触发后，调用此方法
  EventCallback errorCallback_;	// error事件触发后，调用此方法
};

}
#endif  // MUDUO_NET_CHANNEL_H
