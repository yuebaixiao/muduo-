### 知识点梳理 ###

#### 1，Timestamp.h[cc] ####
只有一个成员变量，表示从标准时间到创建对象时点，经过的微秒数。

- public方法
    - Timestamp(int64_t microSecondsSinceEpoch)：构造方法。
	- swap：交换2个对象的成员变量microSecondsSinceEpoch_。
	- toString：返回string，把微秒换算成秒和微秒表示出来。
	 - 此方法里使用了宏PRId64。在32位系统int64_t的实际类型是long long int，所以printf函数里必须使用lld；而在64位系统里int64_t的实际类型是long int，所以printf函数里必须使用ld，为了写出与平台无关的代码所以使用此宏PRId64.
	  ```c
       # if __WORDSIZE == 64
       #  define __PRI64_PREFIX        "l"
       #  define __PRIPTR_PREFIX       "l"
       # else
       #  define __PRI64_PREFIX        "ll"
       #  define __PRIPTR_PREFIX
       # endif
	   # define PRId64         __PRI64_PREFIX "d"
	   
	  ```
	- toFormattedString:返回string，把微秒，换算成年月日时分秒表示出来。
	- valid：判断成员变量microSecondsSinceEpoch_是否大于0.
	- microSecondsSinceEpoch：返回成员变量microSecondsSinceEpoch_
	- static Timestamp now():静态方法，用当前时间构建Timestamp对象，并返回此对象。使用系统调用gettimeofday(&tv, NULL);得到系统时间
	- static Timestamp invalid()：返回成员变量microSecondsSinceEpoch_为0的Timestamp对象。
- 工具方法
	- operator<:运算符重载
	- operator==:运算符重载
	- timeDifference:返回2个参数时间点之间的秒数
	- addTime:根据参数的timestamp，加上参数的seconds，做成新的Timestamp对象，并返回。

#### TimerQueue.h[cc] ####
定时器队列，按时间排序。使用的数据结构是std::set<std::pair<Timestamp, Timer*>>.set是默认排序的，set里的元素不能使用Timestamp，因为有重复的可能，所以使用pair，加了个Timer*的指针，即使Timestamp相同，指针的值也是不同的。

当定时器到时后，会触发timerfdChannel_里的回调方法，并从定时器队列里删除已经到时间的定时器。使用系统调用timerfd_create来监听定时器。
