#include "poller.h"
#include "channel.h"

Poller::Poller(Eventloop *loop)
    : owenerLoop_(loop)
{
}
bool Poller::hasChannel(Channel *channel) const
{
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}