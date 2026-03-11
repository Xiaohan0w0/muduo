#pragma once

#include <iostream>
#include <string>

// 时间类
class Timestamp
{
public:
    Timestamp();
    // 必须显示转换
    explicit Timestamp(int64_t longtime);
    static Timestamp now();
    std::string toString() const;
private:
    int64_t longtime;
};