### 知识点梳理 ###

#### EventLoop的新增方法runInLoop和queueInLoop ####
增加这2个函数的目的：让EvenvLoop线程（IO线程)也可以执行用户的回调函数，有了这个功能，就能够轻易地在线程间调配任务。

由于同一个loop*指针，可以在IO线程里使用，也可以暴露给别的线程使用，在别的线程里也可以往IO线程里添加事件回调函数。

通过使用：loop->runInLoop或者loop->queueInLoop，如果是在IO线程里调用，则立即执行回调函数，如果不是，这些函数会加入到IO线程的事件队列pendingFunctors_里，然后唤醒IO线程。

为什么需要唤醒？因为IO线程通常会阻塞在poll方法，所以需要给poll方法监听的eventfd里write内容，poll方法就会返回，以达到让唤醒IO线程的目的。

什么时候需要唤醒？分2种情况：
- 在非IO线程调用runInLoop和queueInLoop时，需要唤醒IO线程
- 在IO线程调用runInLoop和queueInLoop时，
    - callingPendingFunctors_为true，需要唤醒IO线程
	- callingPendingFunctors_为false，也就是在执行执行用户回调函数时，用户的回调函数又调用了queueInLoop时，不需要唤醒IO线程

#### EventLoop的变更方法quit ####
增加了，不在IO进程调用quit时，唤醒IO进程的逻辑。

#### EventLoop的定时器方法的改进 ####
改进前，只能在IO进程里设置定时器，也就是只能在IO进程调用设置定时器方法(runAt,runAfter,runEvery）。

改进后，可以在非IO进程设置定时器。

实现方法：因为添加了runInLoop，所以在addTimer里掉用runInLoop，注册定时器回调就号了。

#### EventLoopThread类 ####
IO线程不一定是主线程，用EventLoopThread类去创建运行EventLoop::loop的进程。调用EventLoopThread的startThread方法启动IO线程（在此线程里调用loop方法），并返回EventLoop对象的指针。返回的指针在主线程可以使用。

因为是在子线程里创建EventLoop对象，所以主线程要等待子线程创建好EventLoop对象后，才能返回给调用侧EventLoop对象的指针，所以使用到了条件变量和互斥锁。
