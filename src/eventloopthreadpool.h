#pragma once
#include "noncopyable.h"

#include <functional>
#include <vector>
#include <memory>

class Eventloop;
class EventLoopThread;

class EventloopThreadPool:Noncopyable
{
public:
    using ThreadInitCallback = std::function<void(Eventloop*)>;
    EventloopThreadPool(Eventloop *baseLoop, const std::string &nameArg);
    ~EventloopThreadPool();
    void setThreadNum(int numThreads){numThreads_ = numThreads;}
    void start(const ThreadInitCallback &cb = ThreadInitCallback());
    // 如果工作在多线程中，baseloop_默认在轮询的方式分配channel给subloop
    Eventloop* getNextLoop();
    std::vector<Eventloop*> getAllLoops();
    bool started() const{return started_;}
    const std::string name(){return name_;}
private:
    Eventloop *baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<Eventloop*> loops_;
};