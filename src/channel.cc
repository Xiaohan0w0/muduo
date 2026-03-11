#include "channel.h"
#include "eventloop.h"
#include "logger.h"
#include <sys/poll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(Eventloop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
{
}
Channel::~Channel()
{
}

// fd得到poller通知以后，处理事件的
void Channel::handleEvent(Timestamp receiveTime)
{
    // 弱引用 + 强引用 生命周期管理逻辑
    // 让 Channel 与某个 “所有者对象”（如 TcpConnection）绑定，确保事件处理时所有者对象未被销毁
    std::shared_ptr<void> guard;
    // 如果当前 Channel 被绑定到其他对象（tied_ 是绑定标志）
    if (tied_)
    {
        // 尝试从弱引用 tie_ 升级为强引用：成功则说明被绑定对象仍存活
        guard = tie_.lock();
        // 强引用升级成功（被绑定对象存活）
        if (guard)
        {
            // 安全处理事件（此时被绑定对象不会被销毁）
            handleEventWithGuard(receiveTime);
        }
        // 若升级失败（被绑定对象已销毁），则不执行事件处理（避免野指针访问）
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

// 根据poller通知的channel发生的具体事件，有channel负责调用具体的回调操作 __FUNCTION__ __LINE__
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d", revents_);

    // 文件描述符（fd）发生了挂断事件（对端关闭 / 网络断开），且内核缓冲区中没有任何可读数据
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        if (closeCallback_)
        {
            closeCallback_();
        }
    }
    if (revents_ | POLLERR)
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }
    if (revents_ & (POLLIN | POLLPRI))
    {
        if (readCallback_)
        {
            readCallback_(receiveTime);
        }
    }
    if (revents_ & POLLOUT)
    {
        if (writeCallback_)
        {
            writeCallback_();
        }
    }
}

// channel的tie方法什么时候调用过
void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

// 当改变channel所表示fd的events事件后，update负责在poller里面更改fd相应的事件epoll_ctl
// EventLoop => ChannelList Poller
void Channel::update()
{
    // 通过channel所属的EventLoop，调用poller的相应方法，注册fd的events事件
    // loop_->updateChannel(this);
}

// 在channel所属的EventLoop中，把当前的channel删除掉
void Channel::remove()
{
    // loop_->removeChannel(this);
}