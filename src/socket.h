#pragma once
#include "noncopyable.h"
class Inetaddress;

class Socket:Noncopyable
{
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();
    int fd() const { return sockfd_; }
    void bindAdderess(const Inetaddress &localaddr);
    void listen();
    int accept(Inetaddress *peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    const int sockfd_;
};