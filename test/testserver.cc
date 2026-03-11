#include <mymuduo/TcpServer.h>
#include <mymuduo/Logger.h>
#include <mymuduo/EventLoop.h>
#include <mymuduo/InetAddress.h>
#include <mymuduo/Buffer.h>
#include <mymuduo/Timestamp.h>
#include <string>
#include <functional>

class EchoServer
{
public:
    EchoServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg)
        : loop_(loop),
        server_(new TcpServer(loop, listenAddr, nameArg))
    {
        // 注册回调函数
        server_->setConnectionCallback([this](const TcpConnectionPtr &conn)
                                    { this->onConnection(conn); });
        server_->setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
                                    { this->onMessage(conn, buf, time); });
        // 设置合适的 loop 线程数量 loopthread
        server_->setThreadNum(3);
    }
    void start()
    {
        server_->start();
    }
private:
    // 连接回调
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO("Connection UP : %s", conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("Connection DOWN : %s", conn->peerAddress().toIpPort().c_str());
        }
    }
    // 读写回调
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
    {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown(); // 写端   EPOLLHUP =》 closeCallback_
    }

    EventLoop *loop_;
    TcpServer *server_;
};

int main(int argc, char **argv)
{
    EventLoop loop;
    InetAddress listenAddr(8000);
    EchoServer server(&loop, listenAddr, "EchoServer-01"); // Acceptor non-blocking listenfd
    server.start(); // listen  loopthread  listenfd => acceptChannel => mainLoop =>
    loop.loop(); // 启动 mainLoop 的底层 Poller

    return 0;
}