#!/bin/bash
set -e

if [ ! -d "build" ]; then
    mkdir build
fi

rm -rf build/*

cd build && 
    cmake .. &&
    make

# 回到项目根目录
cd ..

# 把头文件拷贝到 /usr/include/mymuduo so库拷贝到 /usr/lib PATH

if [ ! -d /usr/include/mymuduo ]; then
    mkdir /usr/include/mymuduo
fi

# 安装头文件（从 src 目录拷贝）
cp src/*.h /usr/include/mymuduo/
# 安装动态库
cp lib/libmymuduo.so /usr/lib

ldconfig