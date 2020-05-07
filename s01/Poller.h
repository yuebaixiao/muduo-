// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_NET_POLLER_H
#define MUDUO_NET_POLLER_H

#include <map>
#include <vector>

#include "datetime/Timestamp.h"
#include "EventLoop.h"

struct pollfd;

namespace muduo
{

class Channel;			// 由于在此头文件里，没有使用Channel的详细信息，只使用了Channel声明。
				// 所以可以使用前向声明，目的是不include：Channel.h。
				// 在头文件里，尽量不include别的头文件。

///
/// IO Multiplexing with poll(2).
///
/// This class doesn't own the Channel objects.
class Poller : boost::noncopyable
{
 public:
  typedef std::vector<Channel*> ChannelList; // 创建一个类型

  Poller(EventLoop* loop);
  ~Poller();

  /// Polls the I/O events.
  /// Must be called in the loop thread.
  Timestamp poll(int timeoutMs, ChannelList* activeChannels);

  /// Changes the interested I/O events.
  /// Must be called in the loop thread.
  void updateChannel(Channel* channel); // 通过参数channle更新pollfds_

  void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

 private:
  void fillActiveChannels(int numEvents,
                          ChannelList* activeChannels) const; // poll(2)返回后，把更新了的pollfds_，存放到activeChannels里。

  typedef std::vector<struct pollfd> PollFdList;
  typedef std::map<int, Channel*> ChannelMap;

  EventLoop* ownerLoop_;	// 所属的EventLoop对象
  PollFdList pollfds_;		// poll(2)使用的结构体的vector
  ChannelMap channels_;		// 里面放的是fd和Channel的对应关系
};

}
#endif  // MUDUO_NET_POLLER_H
