#include "image.h"
#include "parser.h"
#include "sandpile.h"

void SetOptions(std::deque<std::deque<uint64_t>>& sandpile, ConsoleParams options) {
    SetLengthWidth(sandpile, options.length, options.width);
    SetValues(sandpile, options.input);
}

int main(int argc, char* argv[]) {
    ConsoleParams options = ParseConsole(argc, argv);
    std::deque<std::deque<uint64_t>> matrix;
    SetOptions(matrix, options);
    SandCollapseCycle(matrix, options.max_iter, options.freq, options.output);

    return 0;
}