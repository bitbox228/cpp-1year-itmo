#include <iostream>
#include <string>

struct ConsoleParams {
    uint16_t length;
    uint16_t width;

    std::string input;
    std::string output;

    uint64_t max_iter = 1;
    uint64_t freq = 1;
};

ConsoleParams ParseConsole(int argc, char* argv[]);