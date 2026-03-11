#include "inetaddress.h"
#include <strings.h>
#include <string.h>

// 初始化IP地址和端口号并转换为网络字节序
Inetaddress::Inetaddress(uint16_t port, std::string ip)
{
    // 清空 addr_ 结构体的所有字节，避免脏数据
    bzero(&addr_, sizeof(addr_));
    // 指定地址族为 IPv4 (AF_INET 表示 IPv4 , AF_INET6 表示 IPv6)
    addr_.sin_family = AF_INET;
    // 端口号转换为网络字节序 主机本地字节序的端口号转换为网络字节序(大端序) host to net
    addr_.sin_port = htons(port);
    // IP 地址转换为网络字节序 点分十进制的IP字符串转换为32位整数的网络字节序
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}
Inetaddress::Inetaddress(const sockaddr_in &addr)
{
    this->addr_ = addr_;
}
std::string Inetaddress::toIp() const
{
    // 将网络字节序转化成本地字节序
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}
std::string Inetaddress::toIpPort() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    size_t end = strlen(buf);
    // net to host
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}
uint16_t Inetaddress::toPort() const
{
    return ntohs(addr_.sin_port);
}
const sockaddr_in *Inetaddress::getSockAddr() const
{
    return &addr_;
}

// #include <iostream>
// int main()
// {
//     Inetaddress addr(8080);
//     std::cout << addr.toIpPort() << std::endl;
//     std::cout << addr.getSockAddr() << std::endl;
//     std::cout << addr.toIp() << std::endl;
//     std::cout << addr.toPort() << std::endl;
//     return 0;
// }