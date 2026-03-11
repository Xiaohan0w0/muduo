#include "acceptor.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "logger.h"

static int createNonblocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
}
Acceptor::Acceptor(Eventloop *loop, const Inetaddress &listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(createNonblocking())
    , acceptChannel_(loop_, acceptSocket_.fd())
    , listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAdderess(listenAddr); // bind
    // TcpServer::start() Acceptor::listen() 有新用户连接，要执行一个回调 coonfd->channel->subloop
    acceptChannel_.setReadCallback([this](Timestamp){ handleRead(); });
}
Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    // 设置监听状态为 true
    listenning_ = true;
    acceptSocket_.listen();// listen
    // 当有新用户连接时，会执行回调
    acceptChannel_.enableReading();
}

// listenfd 有事件发生了，就有新用户连接了
void Acceptor::handleRead()
{
    sockaddr_in addr{};
    Inetaddress peerAddr(addr);
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        if (newConnectionCallback_)
        {
            // 轮询找到 subloop，唤醒，分发当前的新客户端的 channel
            newConnectionCallback_(connfd, peerAddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept error:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE)
        {
            LOG_INFO("%s:%s:%d sockfd reached limit \n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}
