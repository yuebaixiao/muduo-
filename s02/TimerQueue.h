// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include <set>
#include <vector>

#include <boost/noncopyable.hpp>

#include "datetime/Timestamp.h"
#include "thread/Mutex.h"
#include "Callbacks.h"
#include "Channel.h"

namespace muduo
{

class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
class TimerQueue : boost::noncopyable
{
 public:
  TimerQueue(EventLoop* loop);
  ~TimerQueue();

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe. Usually be called from other threads.
  TimerId addTimer(const TimerCallback& cb,
                   Timestamp when,
                   double interval); // 往队列里添加定时器

  // void cancel(TimerId timerId);

 private:

  // FIXME: use unique_ptr<Timer> instead of raw pointers.
  typedef std::pair<Timestamp, Timer*> Entry; // set里的元素
  typedef std::set<Entry> TimerList;

  // called when timerfd alarms
  void handleRead();		// 定时器到时后，调用此方法
  // move out all expired timers
  std::vector<Entry> getExpired(Timestamp now); // 根据现在时间点的时间，计算出已经到期的定时器，从定时器队列里删除它们，并返回给调用方。
  void reset(const std::vector<Entry>& expired, Timestamp now); // 由handleRead方法调用此方法。某个定时器到时后，需要使用timerfd_settime系统调用，来定时队列里的下一个时间。

  bool insert(Timer* timer);	// 由handleRead方法调用此方法。某个定时器到时后，需要使用timerfd_settime系统调用，来定时队列里的下一个时间。

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  // Timer list sorted by expiration
  TimerList timers_;		// 定时器队列
};

}
#endif  // MUDUO_NET_TIMERQUEUE_H
