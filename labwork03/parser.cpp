#include "parser.h"

ConsoleParams ParseConsole(int argc, char* argv[]) {
    ConsoleParams options;
    for (size_t i = 1; i < argc; i++) {
        if ((strlen(argv[i]) > 0) && (argv[i][0] == '-')) {
            if ((strcmp(argv[i], "-l") == 0) || (strcmp(argv[i], "--length") == 0)) {
                options.length = static_cast<uint16_t>(std::strtoul(argv[++i], nullptr, 0));
            } else if ((strcmp(argv[i], "-w") == 0) || (strcmp(argv[i], "--width") == 0)) {
                options.width = static_cast<uint16_t>(std::strtoul(argv[++i], nullptr, 0));
            } else if ((strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "--input") == 0)) {
                options.input = argv[++i];
            } else if ((strcmp(argv[i], "-o") == 0) || (strcmp(argv[i], "--output") == 0)) {
                options.output = argv[++i];
            } else if ((strcmp(argv[i], "-m") == 0) || (strcmp(argv[i], "--max-iter") == 0)) {
                options.max_iter = std::stoull(argv[++i]);
            } else if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--freq") == 0)) {
                options.freq = std::stoull(argv[++i]);
            }
        }
    }

    return options;
}