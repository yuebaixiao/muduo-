### 知识点梳理 ###

#### 1，Timestamp.h[cc] ####
只有一个成员变量，表示从标准时间到创建对象时点，经过的微秒数。

- public方法
    - Timestamp(int64_t microSecondsSinceEpoch)：构造方法。
	- swap：交换2个对象的成员变量microSecondsSinceEpoch_。
	- toFormattedString:返回string，把微秒，换算成年月日时分秒表示出来。
	- valid：判断成员变量microSecondsSinceEpoch_是否大于0.
	- microSecondsSinceEpoch：返回成员变量microSecondsSinceEpoch_
	- static Timestamp now():静态方法，用当前时间构建Timestamp对象，并返回此对象。使用系统调用gettimeofday(&tv, NULL);得到系统时间
	- static Timestamp invalid()：返回成员变量microSecondsSinceEpoch_为0的Timestamp对象。
	- toString：返回string，把微秒换算成秒和微秒表示出来。
	  此方法里使用了宏PRId64。在32位系统int64_t的实际类型是long long int，所以printf函数里必须使用lld；而在64位系统里int64_t的实际类型是long int，所以printf函数里必须使用ld，为了写出与平台无关的代码所以使用此宏PRId64.
	 
``` c
       # if __WORDSIZE == 64
       #  define __PRI64_PREFIX        "l"
       #  define __PRIPTR_PREFIX       "l"
       # else
       #  define __PRI64_PREFIX        "ll"
       #  define __PRIPTR_PREFIX
       # endif
	   # define PRId64         __PRI64_PREFIX "d"

```


- 工具方法
	- operator<:运算符重载
	- operator==:运算符重载
	- timeDifference:返回2个参数时间点之间的秒数
	- addTime:根据参数的timestamp，加上参数的seconds，做成新的Timestamp对象，并返回。

#### 2,TimerQueue.h[cc] ####
定时器队列，按时间排序。使用的数据结构是std::set<std::pair<Timestamp, Timer*>>.set是升序排序的，set里的元素不能使用Timestamp，因为有重复的可能，所以使用pair，加了个Timer*的指针，即使Timestamp相同，指针的值也是不同的。

当定时器到时后，会触发timerfdChannel_里的回调方法，并从定时器队列里删除已经到时间的定时器。使用系统调用timerfd_create来监听定时器。

- 成员变量：
    - loop_:所属的EventLoop对象
	- timerfd_:使用timerfd_create系统调用创建的fd
	timerfdChannel_:包装timerfd_的Channel对象
	TimerList timers_:定时器队列
- public方法：
	- TimerQueue(EventLoop* loop)：构造方法
	- addTimer:往队列里添加定时器。定时器包含到时时间，到时的回调函数。如果参数interval大于0，则每过interval秒回调一次回调函数；如果等于0，回调函数只执行一次。注意调用此方法的线程必须是创建loop_的线程
- private方法：
    - handleRead:定时器到时间后，会执行此方法。实现方式是在构造方法里，timerfdChannel_.setReadCallback(&TimerQueue::handleRead, this)).注意调用此方法的线程必须是创建loop_的线程
	- getExpired:根据现在时间点的时间，计算出已经到期的定时器，从定时器队列里删除它们，并返回给调用方。
	实现方式是先用当前时间创建一个哨兵(iterator)，然后使用timers_.lower_bound(sentry)得到第一个大于哨兵时间的iterator(it),然后使用std::copy填充expired，注意这里使用了back_inserter。因为std::copy不能自动增大expired的空间，所以必须使用back_inserter。
	- insert:把参数Timer指针插入到定时器队列，如果它是队列里的第一个元素（最先到时的定时器）的话，则返回true。addTimer方法调用insert，如果发现返回值是true，则用此Timer，调整timerfd_。
	- reset:由handleRead方法调用此方法。某个定时器到时后，需要使用timerfd_settime系统调用，来定时队列里的下一个时间。如果不是重复的定时器则释放Timer指针;如果是重复的定时器，则使用当前时间创建一个新的定时器，并加入到定时器队列里。然后再把定时器队列里的第一个元素作为下一个定时器。

- 工具方法：
    - createTimerfd：执行了timerfd_create系统调用。timerfd_由此方法创建。
	- readTimerfd:定时器到时后，调用此方法。此方法调用read从timerfd_里读数据。读出的值是到期的次数。参看：TIMERFD_CREATE(2)
	- resetTimerfd：调用howMuchTimeFromNow后，调用timerfd_settime，以到达重新设置定时器的到时时间。
	- howMuchTimeFromNow：构建一个struct timespec返回给调用方。调用方使用此结构体，设置timerfd_settime系统调用的第三个参数，第三个参数的类型是结构体itimerspec。
	
``` c
           struct timespec {
               time_t tv_sec;                /* Seconds */
               long   tv_nsec;               /* Nanoseconds */
           };

           struct itimerspec {
               struct timespec it_interval;  /* Interval for periodic timer */
               struct timespec it_value;     /* Initial expiration */
           };

```

#### 3,EventLoop.h[cc] ####
有了TimerQueue后，EventLoop.h里新增了3个高级的定时器，分别是runAt，runAfter，runEvery。

- 新增的成员变量：
    - std::unique_ptr<TimerQueue> timerQueue_;
	
- public方法：
    - runAt:定一个具体时间的定时器
	- runAfter:根据当前时间，几秒后响的定时器。
	- runEvery:定义一个重复的定时器（每几秒钟响一次）
	
``` c
TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
  return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(cb, time, interval);
}

```

#### test4.cc ####
使用了EventLoop里的runAt，runAfter，runEvery方法
