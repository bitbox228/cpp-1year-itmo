cmake_minimum_required(VERSION 3.12)

project(
        PoolAllocator
        VERSION 1.2
)

set(CMAKE_CXX_STANDARD 20)

add_executable(PoolAlloc allocator.h main.cpp)