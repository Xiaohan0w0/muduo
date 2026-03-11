#include "eventloopthread.h"
#include "eventloop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
    : loop_(nullptr)
    , exiting_(false)
    , thread_([this](){ this->threadFunc(); }, name)
    ,mutex_()
    ,cond_()
    ,callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

Eventloop *EventLoopThread::startLoop()
{
    thread_.start();// 启动底层的新线程
    Eventloop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr)
        {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc()
{
    Eventloop loop;// 创建一个独立的eventloop，和上面的线程是一一对应的，one loop per thread
    if(callback_)
    {
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    // 启动loop
    loop.loop();// eventloop loop -> poller.poll 
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}