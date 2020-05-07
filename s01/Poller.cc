// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "Poller.h"

#include "Channel.h"
#include "logging/Logging.h"

#include <assert.h>
#include <poll.h>

using namespace muduo;

Poller::Poller(EventLoop* loop)
  : ownerLoop_(loop)
{
}

Poller::~Poller()
{
}

Timestamp Poller::poll(int timeoutMs, ChannelList* activeChannels)
{
  // XXX pollfds_ shouldn't change
  int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
  Timestamp now(Timestamp::now()); // 得到当前server上的时间。
  if (numEvents > 0) {		   // poll(2)正常返回了
    LOG_TRACE << numEvents << " events happended";
    fillActiveChannels(numEvents, activeChannels);
  } else if (numEvents == 0) {	// poll(2)在指定timeoutMs时间内，没有fd发生事件。
    LOG_TRACE << " nothing happended";
  } else {			// poll(2)发生错误了
    LOG_SYSERR << "Poller::poll()";
  }
  return now;
}

void Poller::fillActiveChannels(int numEvents,
                                ChannelList* activeChannels) const
{
  for (PollFdList::const_iterator pfd = pollfds_.begin();
      pfd != pollfds_.end() && numEvents > 0; ++pfd) // 遍历pollfds_的所有元素
  {
    if (pfd->revents > 0)	// 只有revents大于0的fd,才是有事件发生的fd
    {
      --numEvents;		// 为了尽快退出循环
      ChannelMap::const_iterator ch = channels_.find(pfd->fd); // 找到此fd对应的Channle对象
      assert(ch != channels_.end());
      Channel* channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->set_revents(pfd->revents); // 把pfd里的revent的值，设置到Channel对象的revents上
      // pfd->revents = 0;
      activeChannels->push_back(channel); // 把channel放入activeChannels里
    }
  }
}

void Poller::updateChannel(Channel* channel) // 通过参数channel更新pollfds_和channels_
{
  assertInLoopThread();		// 调用此函数的线程必须是EventLoop所在的线程。
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
  if (channel->index() < 0) {	// 此channel还没有加入到pollfds_里
    // a new one, add to pollfds_
    assert(channels_.find(channel->fd()) == channels_.end());
    struct pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    pollfds_.push_back(pfd);	// 根据channel创建pollfd，然后把pollfd放入到pollfds_里。
    int idx = static_cast<int>(pollfds_.size())-1; // 由于是放入到pollfds_的最后，所以用这种方式得到此channle在pollfds_里的位置
    channel->set_index(idx);			   // 设置channel在pollfds_里的index
    channels_[pfd.fd] = channel;		   // 把此channel加入到channels_里
  } else {					   // 此channel已经加入到了pollfds_里了
    // update existing one
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    int idx = channel->index();	// 根据channel的index，找到它在pollfds_里的位置
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    struct pollfd& pfd = pollfds_[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -1);
    pfd.events = static_cast<short>(channel->events()); // 根据channel的值，更新pfd是events
    pfd.revents = 0;					// 更新为0
    if (channel->isNoneEvent()) {			// 如果不想让poll关注此channel里的fd里的话，把fd设置为-1
      // ignore this pollfd
      pfd.fd = -1;
    }
  }
}

