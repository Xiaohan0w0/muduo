#pragma once
#include "noncopyable.h"
#include "thread.h"
#include <functional>
#include "eventloop.h"
#include <mutex>
#include <condition_variable>

class Eventloop;

class EventLoopThread:Noncopyable 
{ 
public:
    using ThreadInitCallback = std::function<void(Eventloop *)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),const std::string &name=std::string());
    ~EventLoopThread();

    Eventloop *startLoop();
private:
    void threadFunc();

    Eventloop *loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};