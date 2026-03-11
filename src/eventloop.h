#pragma once
#include "noncopyable.h"
#include <functional>
#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include "timestamp.h"
#include "currentthread.h"

class Channel;
class Poller;

// 事件循环类 主要包含了两个大模块 channel(fd+事件+回调) poller(epoll的抽象)
class Eventloop : Noncopyable
{
public:
    using Functor = std::function<void()>;
    Eventloop();
    ~Eventloop();

    // 开启事件循环
    void loop();
    // 退出事件循环
    void quit();
    Timestamp pollReturnTime() const { return pollReturnTime_; }
    // 在当前loop中执行cb
    void runInLoop(Functor cb);
    // 把cb放入队列中，唤醒loop所在的线程，执行cb
    void queueInLoop(Functor cb);
    // 用来唤醒loop所在的线程
    void wakeup(); // main reactor唤醒sub reactor

    // 通过Eventloop的方法（调用）-> Poller的方法
    void updateChannel(Channel *channel); // channel调用poller的方法，将channel加入到poller中
    bool hasChannel(Channel *channel);
    void removeChannel(Channel *channel);

    // 判断当前loop是否在自己的线程中
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    void handleRead();        // wake up
    void doPendingFunctors(); // 执行回调

private:
    using ChannelList = std::vector<Channel *>;

    std::atomic_bool looping_; // 原子操作，通过CAS实现
    std::atomic_bool quit_;    // 标志退出事件循环

    const pid_t threadId_;     // 记录当前loop所在线程的id
    Timestamp pollReturnTime_; // poller返回发生事件的channels的时间点
    std::unique_ptr<Poller> poller_;
    int wakeupFd_; // 当mainLoop获取一个新用户的channel时，通过轮询算法选择一个subLoop，通过该成员唤醒subLoop唤醒channel
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    Channel *currentActiveChannel_;

    std::atomic_bool callingPendingFunctors_; // 标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_;    // 存储loop需要执行的所有回调操作
    std::mutex mutex_;                        // 互斥锁 保护vector容器的线程安全操作
};