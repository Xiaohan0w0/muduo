#pragma once

// noncopyable被继承后，派生类可进行构造和析构，不能进行拷贝和赋值
class Noncopyable
{
public:
    Noncopyable(const Noncopyable&) = delete;
    void operator=(const Noncopyable &) = delete;

protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};