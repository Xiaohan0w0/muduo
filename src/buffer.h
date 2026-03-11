#pragma once
#include "noncopyable.h"
#include <vector>
#include <algorithm>
#include <string>

// 数据过多，通过缓存减少系统调用
// 解决tcp粘包问题，加一个数据头，描述数据的长度，截取相应数据包的大小
// prependable bytes(缓冲区预留8字节) -> readable bytes -> writeable bytes
// 0<=readIndex_ <= writeIndex_ <= size
class Buffer : Noncopyable
{
public:
    static const size_t kInitialSize = 1024;
    static const size_t kCheapPrepend = 8;
    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize), readerIndex_(kCheapPrepend), writeIndex_(kCheapPrepend)
    {
    }
    size_t readableBytes() const
    {
        return writeIndex_ - readerIndex_;
    }
    size_t writeableBytes() const
    {
        return buffer_.size() - writeIndex_;
    }
    size_t prependableBytes() const
    {
        return readerIndex_;
    }
    const char *peek()
    {
        return begin() + readerIndex_;
    }
    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }
    // 指针复位
    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writeIndex_ = kCheapPrepend;
    }
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        std::string str(peek(), len);
        retrieve(len);
        return str;
    }
    void ensureWritableBytes(size_t len)
    {
        if (writeableBytes() < len)
        {
            makeSpace(len);
        }
    }
    void append(const char *data, size_t len)
    {
        /*
        std::copy(
        begin() + readerIndex_,   // 1. 源数据开始
        begin() + writeIndex_,    // 2. 源数据结束
        begin() + kCheapPrepend   // 3. 目标位置
        */
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        writeIndex_ += len;
    }
    char* beginWrite()
    {
        return begin() + writeIndex_;
    }
    
    // 扩容函数
    void makeSpace(size_t len)
    {
        // kCheapPrepend  |   reader   |   write    |
        // kCheapPrepend  |                  len               |
        // [预留8][      已读空间    ][    未读数据    ][可写空间]
        //                          ↑                ↑
        //                      readerIndex_     writeIndex_

        if (prependableBytes() + writeableBytes() < len + kCheapPrepend)
        {
            // 扩容，resize是把vector的总长度直接改成n
            buffer_.resize(writeIndex_ + len);
        }
        else
        {
            // 未读区域和已读区域换位置
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,   // 数据从哪开始（有效数据开头）
                      begin() + writeIndex_,    // 数据到哪结束（有效数据结尾）
                      begin() + kCheapPrepend); // 数据要搬到哪去（最前面8字节后）
            readerIndex_ = kCheapPrepend;
            writeIndex_ = readerIndex_ + readable;
        }
    }
    // 从fd上读取数据
    ssize_t readFd(int fd, int *saveErrno);
    // 从fd上写数据
    ssize_t writeFd(int fd, int *saveErrno);

private:
    char *begin()
    {
        return &*buffer_.begin();
    }
    std::vector<char> buffer_;
    // 读指针：下一次要从这里读数据
    size_t readerIndex_;
    // 写指针：下一次要从这里写数据
    size_t writeIndex_;
};