#include "eventloopthreadpool.h"
#include "eventloopthread.h"
#include <memory>
EventloopThreadPool::EventloopThreadPool(Eventloop *baseLoop, const std::string &nameArg)
    : baseLoop_(baseLoop), name_(nameArg), started_(false), numThreads_(0), next_(0)
{
}
EventloopThreadPool::~EventloopThreadPool()
{
}

void EventloopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;
    for (int i = 0; i < numThreads_; i++)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }
    // 整个服务端只有一个线程，运行着baseloop
    if (numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}
// 如果工作在多线程中，baseloop_默认在轮询的方式分配channel给subloop，选择一个subloop
Eventloop *EventloopThreadPool::getNextLoop()
{
    Eventloop *loop = baseLoop_;
    // 轮询
    if (!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if (next_ >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}
std::vector<Eventloop *> EventloopThreadPool::getAllLoops()
{
    if (loops_.empty())
    {
        // vector里面存放一个元素，元素值是baseLoop_
        return std::vector<Eventloop *>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}