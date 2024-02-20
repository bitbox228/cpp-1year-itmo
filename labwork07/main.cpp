#include "ID3v2.h"

#include <iostream>

int main(int argc, char *argv[]) {

    if (argc <= 1) {
        std::cerr << "No arguments.";
        return 0;
    }

    std::string path = argv[1];
    ID3v2 file;
    file.ParseFile(path);

    bool all = false;
    std::vector<std::string> frames;
    for (size_t i = 2; i < argc; i++) {
        if (strlen(argv[i]) == 4) {
            frames.emplace_back(argv[i]);
        } else if (strcmp(argv[i], "--all") == 0 || strcmp(argv[i], "-a") == 0) {
            all = true;
            break;
        }
    }

    if (all) {
        file.PrintFrames();
    } else if (!frames.empty()) {
        file.PrintFrames(frames);
    } else {
        std::cerr << "No frames to print.";
    }

    return 0;
}