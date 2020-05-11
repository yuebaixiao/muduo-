// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREAD_H
#define MUDUO_BASE_THREAD_H

#include <muduo/base/Atomic.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Types.h>

#include <functional>
#include <memory>
#include <pthread.h>

namespace muduo
{

class Thread : noncopyable
{
 public:
  typedef std::function<void ()> ThreadFunc;

  explicit Thread(ThreadFunc, const string& name = string());
  // FIXME: make it movable in C++11
  ~Thread();

  void start();
  int join(); // return pthread_join()

  bool started() const { return started_; }
  // pthread_t pthreadId() const { return pthreadId_; }
  pid_t tid() const { return tid_; }
  const string& name() const { return name_; }

  static int numCreated() { return numCreated_.get(); }

 private:
  void setDefaultName();

  bool       started_;		// 线程是否启动了
  bool       joined_;		// 线程释放joined了
  pthread_t  pthreadId_;	// pthread_create等系统调用使用它
  pid_t      tid_;		// 创建的线程的进程id
  ThreadFunc func_;		// 要启动的线程要运行的函数
  string     name_;		// 线程的名字
  CountDownLatch latch_;	// 倒计时锁，计数器是1。目的是等待runInThread函数里，把tid的值设置完毕。如果没有 latch_，会有 race condition，即调用 Thread::tid() 的时候线程还没有启动，结果返回初值 0。

  static AtomicInt32 numCreated_; // 由于是静态的，所以是个全局变量，里面保存的是一共创建了多少个线程。
};

}  // namespace muduo
#endif  // MUDUO_BASE_THREAD_H
