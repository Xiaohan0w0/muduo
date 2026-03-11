#pragma once
#include "noncopyable.h"
#include "inetaddress.h"
#include "acceptor.h"
#include "tcpconnection.h"
#include "eventloopthreadpool.h"
#include "callbacks.h"
#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <unordered_map>

// 对外的服务器编程使用的类
class Tcpserver : Noncopyable
{
public:
    using ThreadInitCallback = std::function<void(Eventloop *)>;

    enum Option
    {
        kNoReusePort,
        kReusePort
    };

    Tcpserver(Eventloop *loop, const Inetaddress &listenAddr,const std::string &nameArg,Option option = kNoReusePort);
    ~Tcpserver();
    // 设置底层subloop的个数
    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback &cb){threadInitCallback_ = cb;}
    void setConnetionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback&cb){messageCallback_ = cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback&cb){writeCompleteCallback_ = cb;}
    // 开启服务器监听
    void start();

private:
    void newConnection(int sockfd, const Inetaddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    Eventloop *loop_; // baseloop 用户定义的loop

    const std::string name_;
    const std::string ipPort_;

    std::unique_ptr<Acceptor> acceptor_; // 运行mianloop，任务就是监听新连接事件

    std::shared_ptr<EventloopThreadPool> threadPool_; // one loop per thread

    // 新连接回调
    ConnectionCallback connectionCallback_;
    // 读写回调
    MessageCallback messageCallback_;
    // 消息发送完回调
    WriteCompleteCallback writeCompleteCallback_;
    // 线程初始化回调
    ThreadInitCallback threadInitCallback_;

    std::atomic_int started_;

    int nextConnId_;
    // 保存所有连接
    ConnectionMap connections_;
};