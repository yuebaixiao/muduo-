// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include "thread/Thread.h"

namespace muduo
{

class EventLoop : boost::noncopyable
{
 public:

  EventLoop();
  ~EventLoop();

  void loop();

  void assertInLoopThread()
  {
    if (!isInLoopThread())
    {
      abortNotInLoopThread();
    }
  }
  // CurrentThread::tid()返回调用CurrentThread::tid()方法的线程的线程id
  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

 private:

  void abortNotInLoopThread();	// 终止当前线程

  bool looping_; /* atomic */   //在构造方法里设置成false，在loop方法里设置成true
  const pid_t threadId_;	// 保存的是创建EventLoop对象的线程的线程id，在构造方法里使用：threadId_(CurrentThread::tid()。
};

}

#endif  // MUDUO_NET_EVENTLOOP_H
