#include "tcpserver.h"
#include "eventloop.h"
#include "logger.h"
#include <functional>

Eventloop *CheckLoopNotNull(Eventloop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainloop is null \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

Tcpserver::Tcpserver(Eventloop *loop, const Inetaddress &listenAddr, const std::string &nameArg, Option option)
    : loop_(CheckLoopNotNull(loop)), ipPort_(listenAddr.toIpPort()), name_(nameArg), acceptor_(new Acceptor(loop_, listenAddr, option)),threadPool_(new EventloopThreadPool(loop_, name_)), connectionCallback_(),messageCallback_(), nextConnId_(1)
{
    acceptor_->setNewConnectionCallback([this](int sockfd, const Inetaddress &peerAddr)
                                        { this->newConnection(sockfd, peerAddr); });
}
void Tcpserver::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}
void Tcpserver::start()
{
    // 防止一个tcpsever被start()多次
    if (started_++ == 0)
    {
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop([this]()
                        { acceptor_->listen(); });
        // loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
        // 智能指针.get()方法获取指向那个对象的地址
    }
}
void Tcpserver::newConnection(int sockfd, const Inetaddress &peerAddr)
{

}
void Tcpserver::removeConnection(const TcpConnectionPtr &conn)
{}
void Tcpserver::removeConnectionInLoop(const TcpConnectionPtr &conn)
{

}