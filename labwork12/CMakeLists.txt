cmake_minimum_required(VERSION 3.12)

project(
        MyCoolDataBase
        VERSION 1.1
)

set(CMAKE_CXX_STANDARD 23)

link_directories(lib)

add_subdirectory(lib)
add_subdirectory(bin)

enable_testing()
add_subdirectory(tests)