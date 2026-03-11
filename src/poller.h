#pragma once
#include <vector>
#include <unordered_map>
#include "noncopyable.h"
#include "timestamp.h"


class Channel;
class Eventloop;

// muduo库中多路事件分发器的核心IO复用模块
class Poller
{
public:
    using ChannelList = std::vector<Channel *>;
    Poller(Eventloop *loop);

    virtual ~Poller();
    // 给所有IO复用保留统一的接口
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    // 判断某个channel是否在poller中
    bool hasChannel(Channel *channel) const;

    // 创建默认的IO复用，Eventloop可以通过该接口获取默认的IO复用的具体实现
    static Poller *newDefaultPoller(Eventloop *loop);
protected:
    // map的key：sockfd value: sockfd所属的Channel通道类型
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap channels_;

private:
    // 定义Poller所属的事件循环Eventloop
    Eventloop *owenerLoop_;
};