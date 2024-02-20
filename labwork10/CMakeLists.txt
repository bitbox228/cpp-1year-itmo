cmake_minimum_required(VERSION 3.25)
project(WeatherWidget)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
        WeatherForecast.cpp
        WeatherForecast.h
        dotenv.cpp
        dotenv.h
        main.cpp)

if (WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

include(FetchContent)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 742fb6f1a5ed40efe8f3a994e0559c6aff52f2d7)

FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG 6af826d0bdb55e4b69e3ad817576745335f243ca)

FetchContent_Declare(ftxui GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
        GIT_TAG 898b8d672ef2cea386048b79276db26867997bae)

set(LIBENVPP_INSTALL ON CACHE BOOL "" FORCE)
FetchContent_Declare(libenvpp
        GIT_REPOSITORY https://github.com/ph3at/libenvpp.git
        GIT_TAG v1.0.0
        )
FetchContent_MakeAvailable(libenvpp)

FetchContent_MakeAvailable(cpr)
FetchContent_MakeAvailable(nlohmann_json)
FetchContent_MakeAvailable(ftxui)

target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE ftxui::dom ftxui::component ftxui::screen)