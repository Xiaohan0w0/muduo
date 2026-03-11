#pragma once
#include "socket.h"
#include "channel.h"
#include "noncopyable.h"
#include "inetaddress.h"
#include <functional>

class Eventloop;
class Inetaddress;

class Acceptor : Noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const Inetaddress &)>;
    Acceptor(Eventloop *loop, const Inetaddress &listenAddr, bool reuseport);
    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback_ = std::move(cb);
    }
    bool listenning() const
    {
        return listenning_;
    }
    void listen();

private:
    void handleRead();
    Eventloop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};