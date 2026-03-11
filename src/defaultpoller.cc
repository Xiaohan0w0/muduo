#include "poller.h"
#include "epollpoller.h"
#include <stdlib.h>

Poller *newDefaultPoller(Eventloop *loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return nullptr;// 生成poll的实例
    }
    else
    {
        return new EpollPoller(loop);// 生成epoll的实例
    }
}