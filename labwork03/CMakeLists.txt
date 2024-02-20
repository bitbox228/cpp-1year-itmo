cmake_minimum_required(VERSION 3.12)

project(
        SandPile
        VERSION 1.0
)

set(CMAKE_CXX_STANDARD 17)

add_executable(SandPile main.cpp image.h image.cpp parser.h parser.cpp sandpile.h sandpile.cpp)