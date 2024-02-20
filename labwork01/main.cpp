#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

class ConsoleOptions {
public:
    bool lines_ = false, words_ = false, bytes_ = false, chars_ = false;

    void AllOptionsToTrue() {
        lines_ = true;
        words_ = true;
        bytes_ = true;
        chars_ = true;
    }

    bool IsAllOptionsFalse() {
        if (words_ + bytes_ + lines_ + chars_ == 0) {
            return true;
        } else {
            return false;
        }
    }
};

size_t BytesCount(std::ifstream& file) {
    char current;
    size_t bytes = 0;
    while (file.get(current)) {
        bytes++;
    }
    return bytes;
}

size_t WordsCount(std::ifstream& file) {
    file.clear();
    file.seekg(0, std::ios::beg);
    char previous = NULL, current = NULL;
    size_t words = 0;
    file.get(previous);
    while (file.get(current)) {
        if (std::isprint(previous) && (previous != ' ') && std::isspace(current)) {
            words++;
        }
        previous = current;
    }
    if (current == NULL) {
        if ((previous != NULL) && (previous != ' ') && (std::isprint(previous))) {
            words++;
        }
    } else if ((current != ' ') && std::isprint(current)) {
        words++;
    }
    return words;
}

size_t LinesCount(std::ifstream& file) {
    file.clear();
    file.seekg(0, std::ios::beg);
    char current;
    size_t lines = 1;
    while (file.get(current)) {
        if (current == '\n') {
            lines++;
        }
    }
    return lines;
}

size_t CharsCount(std::ifstream& file) {
    file.clear();
    file.seekg(0, std::ios::beg);
    char current;
    size_t chars = 0;
    while (file.get(current)) {
        if (std::isprint(current)) {
            chars++;
        }
    }
    return chars;
}

int main(int argc, char *argv[]) {
    ConsoleOptions options;
    std::vector<std::string> filenames;
    for (size_t i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if ((strlen(argv[i]) > 1) && (argv[i][1] == '-')) {
                if (strcmp(argv[i], "--lines") == 0) {
                    options.lines_ = true;
                } else if (strcmp(argv[i], "--words") == 0) {
                    options.words_ = true;
                } else if (strcmp(argv[i], "--bytes") == 0) {
                    options.bytes_ = true;
                } else if (strcmp(argv[i], "--chars") == 0) {
                    options.chars_ = true;
                } else {
                    std::cout << "Options can be only --lines, --words, --bytes, --chars or -l, -c, -w, -m.\n";
                    return 0;
                }
            } else {
                for (size_t j = 1; j < strlen(argv[i]); j++) {
                    if (argv[i][j] == 'l') {
                        options.lines_ = true;
                    } else if (argv[i][j] == 'c') {
                        options.bytes_ = true;
                    } else if (argv[i][j] == 'w') {
                        options.words_ = true;
                    } else if (argv[i][j] == 'm') {
                        options.chars_ = true;
                    } else {
                        std::cout << "Options can be only --lines, --words, --bytes, --chars or -l, -c, -w, -m.\n";
                        return 0;
                    }
                }
            }
        } else {
            filenames.push_back(argv[i]);
        }
    }
    if (options.IsAllOptionsFalse()) {
        options.AllOptionsToTrue();
    }

    std::ifstream file, file_binary;
    std::string filename;
    for (size_t i = 0; i < filenames.size(); i++) {
        filename = filenames[i];
        file.open(filename);
        file_binary.open(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Invalid name of file.\n";
            return 0;
        }
        if (options.lines_) {
            std::cout << LinesCount(file) << ' ';
        }
        if (options.words_) {
            std::cout << WordsCount(file) << ' ';
        }
        if (options.bytes_) {
            std::cout << BytesCount(file_binary) << ' ';
        }
        if (options.chars_) {
            std::cout << CharsCount(file) << ' ';
        }
        std::cout << filename << std::endl;
    }
    return 0;
}