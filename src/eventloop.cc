#include "eventloop.h"
#include "poller.h"
#include "logger.h"
#include "channel.h"
#include <unistd.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <errno.h>

// 防止一个线程创建多个Eventloop
__thread Eventloop *t_loopInThisThread = nullptr;

// 定义默认的Poller IO复用的超时时间
const int kPollTimeMs = 10000;

// 创建wakeupfd，用来notify唤醒subReactor处理新来的channel
// main reactor通过轮询唤醒，subreactor通过eventfd进行通知
int cereateEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("eventfd error:%d \n", errno);
    }
    return evtfd;
}

Eventloop::Eventloop()
    : looping_(false), quit_(false), callingPendingFunctors_(false), threadId_(CurrentThread::tid()), poller_(Poller::newDefaultPoller(this)), wakeupFd_(cereateEventfd()), wakeupChannel_(new Channel(this, wakeupFd_)), currentActiveChannel_(nullptr)
{
    LOG_DEBUG("Eventloop created %p in thread %d \n", this, threadId_);
    if (t_loopInThisThread)
    {
        LOG_FATAL("Another Eventloop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }
    // 设置wakeup得事件类型以及发生事件后得回调操作
    // wakeupChannel_->setReadCallback(std::bind(&Eventloop::handleRead, this));
    wakeupChannel_->setReadCallback([this](Timestamp)
                                    { handleRead(); }); // ReadEventCallback = std::function<void(Timestamp)>;需要参数timestamp
    // 每一个eventloop都将监听wakeupchannel得EPOLLIN读事件了
    wakeupChannel_->enableReading();
}
Eventloop::~Eventloop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void Eventloop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("Eventloop::handleRead() reads %lu bytes instead of 8", n);
    }
}

// 开启事件循环
void Eventloop::loop()
{ 
    looping_ = true;
    quit_ = false;
    LOG_INFO("Eventloop %p start looping \n", this);
    while(!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);    
        for(Channel* Channel: activeChannels_)
        {
            // Poller监听那些channel发生事件了，然后上报给EventLoop，通知channel处理相应的事件
            Channel->handleEvent(pollReturnTime_);
        }
        // 执行当前EventLoop事件循环需要处理的回调操作
        doPendingFunctors();
    }
    LOG_INFO("Eventloop %p stop looping \n", this);
    looping_ = false;
}

// 退出事件循环
void Eventloop::quit()
{
    quit_ = true;
    if (!isInLoopThread())
    {
        wakeup();
    }
}

// 在当前loop中执行cb
void Eventloop::runInLoop(Functor cb)
{

    if (isInLoopThread())// 在当前loop中，执行cb
    {
        cb();
    }
    else// 在非当前loop中，添加cb到pendingFunctors_中，唤醒loop所在的线程，执行cb
    {
        queueInLoop(cb);
    }
}

// 把cb放入队列，唤醒loop所在的线程
void Eventloop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    // 唤醒loop所在的线程
    // || callingPendingFunctors_的意思是：当前loop正在执行回调，但是loop又有了新的回调
    if(!isInLoopThread()||callingPendingFunctors_)
    {
        wakeup();// 唤醒loop所在的线程
    }
}

// 用来唤醒loop所在的线程，向wakeupfd_写入数据，wakeupChannel就发生读事件，当前loop线程就会被唤醒
void Eventloop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("Eventloop::wakeup() writes %lu bytes instead of 8 \n", n);
    }
}

void Eventloop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void Eventloop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool Eventloop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void Eventloop::doPendingFunctors()// 执行回调
{ 
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for (const Functor &functor : functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}