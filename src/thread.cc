#include "thread.h"
#include "currentthread.h"
#include <semaphore.h>

std::atomic_int Thread::numCreated_{0};

// func底层资源直接给成员变量func_，move移动语义
Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false), joined_(false), tid_(0), func_(std::move(func)), name_(name)
{
    setDefaultName();
}
Thread::~Thread()
{
    if (started_ && !joined_)
    {
        thread_->detach(); // thread类提供的设置分离线程的方法
    }
}

void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);

    // 第一步，开启子线程，让他去跑
    thread_ = std::shared_ptr<std::thread>(new std::thread([&]()
        {
        // 子线程先做第一件事：获取自己的线程ID
        tid_= CurrentThread::tid();
        // 信号量 +1 → 告诉主线程：我拿到tid了！
        // 第三步子线程post唤醒主线程
        sem_post(&sem);
        // 开启一个新线程，真正执行业务函数
        func_(); }));

    // 这里必须等待上面新创建的线程的tid值
    // 第二步，主线程阻塞在这里，直到子线程把 sem_post 执行完
    sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty())
    {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}
