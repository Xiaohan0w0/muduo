# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)

# 分别编译 test.cc 和 testserver.cc 为两个独立的可执行文件
add_executable(test test.cc)
add_executable(testserver testserver.cc)
# 链接 mymuduo 动态库和 pthread 库
target_link_libraries(testserver mymuduo pthread)
