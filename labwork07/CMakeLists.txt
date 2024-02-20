cmake_minimum_required(VERSION 3.12)

project(
        ID3_parser
        VERSION 1.2
)

set(CMAKE_CXX_STANDARD 20)

add_executable(Parser main.cpp ID3v2.h ID3v2.cpp Frame.h Frame.cpp)