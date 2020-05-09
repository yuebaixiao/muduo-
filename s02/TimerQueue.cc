// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#define __STDC_LIMIT_MACROS
#include "TimerQueue.h"

#include "logging/Logging.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <boost/bind.hpp>

#include <sys/timerfd.h>

namespace muduo
{
namespace detail
{

int createTimerfd()		// 调用timerfd_create创建timerfd_
{
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, // CLOCK_MONOTONIC:以固定的速率运行，从不进行调整和复位 ,它不受任何系统time-of-day时钟修改的影响
                                 TFD_NONBLOCK | TFD_CLOEXEC); // 非阻塞，exec后关闭此fd
  if (timerfd < 0)
  {
    LOG_SYSFATAL << "Failed in timerfd_create";
  }
  return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
  int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch(); // 取得参数与现在时间点的微秒之差
  if (microseconds < 100)	// 小于100微秒，按100微秒
  {
    microseconds = 100;
  }
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(
      microseconds / Timestamp::kMicroSecondsPerSecond); // 换算成秒
  ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000); // 换算成纳秒
  return ts;
}

void readTimerfd(int timerfd, Timestamp now)
{
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof howmany); // howmany里是定时器到期的次数
  LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
  if (n != sizeof howmany)	// howmany的位数必须和n相同
  {
    LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
  }
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
  // wake up loop by timerfd_settime()
  struct itimerspec newValue;
  struct itimerspec oldValue;
  bzero(&newValue, sizeof newValue);
  bzero(&oldValue, sizeof oldValue);
  newValue.it_value = howMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue); // 重新设置定时器
  if (ret)
  {
    LOG_SYSERR << "timerfd_settime()";
  }
}

}
}

using namespace muduo;
using namespace muduo::detail;

TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_()
{
  timerfdChannel_.setReadCallback(
      boost::bind(&TimerQueue::handleRead, this)); // 设置定时器的回调函数
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  timerfdChannel_.enableReading(); // 把timerfd_加入到poll或epoll里
}

TimerQueue::~TimerQueue()
{
  ::close(timerfd_);		// 关闭fd
  // do not remove channel, since we're in EventLoop::dtor();
  for (TimerList::iterator it = timers_.begin();
      it != timers_.end(); ++it)
  {
    delete it->second;		// 释放Timer指针
  }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb,
                             Timestamp when,
                             double interval)
{
  Timer* timer = new Timer(cb, when, interval);
  loop_->assertInLoopThread();
  bool earliestChanged = insert(timer);

  if (earliestChanged)
  {
    resetTimerfd(timerfd_, timer->expiration()); // 重新设置定时器
  }
  return TimerId(timer);
}

void TimerQueue::handleRead()
{
  loop_->assertInLoopThread();
  Timestamp now(Timestamp::now());
  readTimerfd(timerfd_, now);	// 读timerfd_

  std::vector<Entry> expired = getExpired(now); // 取得到时的定时器

  // safe to callback outside critical section
  for (std::vector<Entry>::iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    it->second->run();		// 执行到期定时器的回调方法
  }

  reset(expired, now);		// 重新设置定时器队列
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
  std::vector<Entry> expired;	// 空的vector
  Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX)); // https://zh.cppreference.com/w/cpp/language/reinterpret_cast 创建哨兵，用当前时间和Timer*的最大值组成pair
  // UINTPTR_MAX是uintptr_t 类型对象的最大值
  TimerList::iterator it = timers_.lower_bound(sentry); // 找到第一个大于哨兵的iterator
  assert(it == timers_.end() || now < it->first);
  std::copy(timers_.begin(), it, back_inserter(expired)); // 拷贝到期的定时器到expired里
  timers_.erase(timers_.begin(), it);			  // 从定时器队列中删除到时的定时器。

  return expired;		// 返回到时的定时器vector
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
  Timestamp nextExpire;

  for (std::vector<Entry>::const_iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    if (it->second->repeat())	// 到时的定时器如果是重复的话，在把它加入到定时器队列，并重新设置到时时间。
    {
      it->second->restart(now);
      insert(it->second);
    }
    else
    {
      // FIXME move to a free list
      delete it->second;	// 不是重复的到时的定时器，所以释放
    }
  }

  if (!timers_.empty())		// 如果队列不为空，取得队列的第一个元素的Timer指针。
  {
    nextExpire = timers_.begin()->second->expiration();
  }

  if (nextExpire.valid())	// 如果nextExpire.valid()大于0，则把队列的第一个元素的Timer设置为最早要到时的定时器
  {
    resetTimerfd(timerfd_, nextExpire);
  }
}

bool TimerQueue::insert(Timer* timer)
{
  bool earliestChanged = false;
  Timestamp when = timer->expiration();
  TimerList::iterator it = timers_.begin();
  if (it == timers_.end() || when < it->first) // 如果队列里第一元素的到时时间在参数Timer之后，则说明参数的定时器会最先到时
  {
    earliestChanged = true;
  }
  std::pair<TimerList::iterator, bool> result =
          timers_.insert(std::make_pair(when, timer));
  assert(result.second);
  return earliestChanged;
}

