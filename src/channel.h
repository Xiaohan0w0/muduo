#pragma once

#include "noncopyable.h"
#include "timestamp.h"

#include <functional>
#include <memory>

class Eventloop;

// fd的事件封装器，绑定了fd+事件+回调，解耦fd与事件；每一个fd对应一个channel由eventloop管理
// 把fd+事件+回调封装成对象，比如listen_fd对应一个channel，conn_fd对应另一个channel，eventloop只管理channel，不直接碰fd
// channel理解为通道，封装了sockfd和感兴趣的event，绑定了poller返回的具体事件
class Channel : Noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;
    Channel(Eventloop *loop, int fd);
    ~Channel();
    // fd得到poller通知以后，处理事件的
    void handleEvent(Timestamp receiveTime);
    // 设置回调函数对象
    // 函数传参，赋值两次拷贝，性能差；利用move将左值形参cb强行转化成右值引用，触发移动构造，只需一次函数传参的拷贝
    void setReadCallback(ReadEventCallback cb) { this->readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { this->writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { this->closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { this->errorCallback_ = std::move(cb); }

    // 防止当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { this->events_ = revt; }

    // 设置fd相应的事件状态
    void enableReading()
    {
        this->events_ |= kReadEvent;
        update();
    }
    void disableReading()
    {
        this->events_ &= ~kReadEvent;
        update();
    }
    void enableWriting()
    {
        this->events_ |= kWriteEvent;
        update();
    }
    void disableWrting()
    {
        this->events_ &= ~kWriteEvent;
        update();
    }
    void disableAll()
    {
        this->events_ = kNoneEvent;
        update();
    }

    // 返回fa当前的事件状态
    bool isNoneEvent() const { return this->events_ == kNoneEvent; }
    bool isWriting() const { return this->events_ & kWriteEvent; }
    bool isReading() const { return this->events_ & kReadEvent; }

    int index() { return this->index_; }
    void set_index(int idx) { this->index_ = idx; }

    // one loop per thread
    Eventloop *ownerLoop() { return this->loop_; }
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    Eventloop *loop_; // 事件循环
    const int fd_;    // fd，poller监听的对象
    int events_;      // 注册fd感兴趣的事件，注册事件
    int revents_;     // poller返回的具体发生的事件，返回事件
    int index_;

    // 指向被绑定对象的弱引用（不影响对象生命周期，仅用于判断对象是否存活）
    std::weak_ptr<void> tie_;
    // channel对象有 tied_成员变量，用于判断当前channel是否已经绑定了其他对象对象
    bool tied_;

    // 因为channel通道里面能够获知fd最终发生的具体事件revents，所以它负责调用具体事件的回调操作
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};