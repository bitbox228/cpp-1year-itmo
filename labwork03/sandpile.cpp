#include "sandpile.h"

void AddLeftColumn(std::deque<std::deque<uint64_t>>& sandpile) {
    uint16_t height = sandpile.size();

    for (uint16_t i = 0; i < height; i++) {
        sandpile[i].push_front(0);
    }
}

void AddRightColumn(std::deque<std::deque<uint64_t>>& sandpile) {
    uint16_t height = sandpile.size();

    for (uint16_t i = 0; i < height; i++) {
        sandpile[i].push_back(0);
    }
}

void AddUpperLine(std::deque<std::deque<uint64_t>>& sandpile) {
    uint16_t width = sandpile[0].size();

    std::deque<uint64_t> line(width, 0);
    sandpile.push_front(line);
}

void AddLowerLine(std::deque<std::deque<uint64_t>>& sandpile) {
    uint16_t width = sandpile[0].size();

    std::deque<uint64_t> line(width, 0);
    sandpile.push_back(line);
}

void SandMove(std::deque<std::deque<uint64_t>>& sandpile, uint16_t i, uint16_t j, bool upperLine, bool leftColumn) {
    sandpile[i + upperLine][j + leftColumn] -= 4;
    sandpile[i + 1 + upperLine][j + leftColumn]++;
    sandpile[i + upperLine][j + 1 + leftColumn]++;
    sandpile[i - 1 + upperLine][j + leftColumn]++;
    sandpile[i + upperLine][j - 1 + leftColumn]++;
}

bool SandCollapse(std::deque<std::deque<uint64_t>>& sandpile, std::deque<std::deque<uint64_t>>& sandpile_copy) {
    uint16_t width = sandpile[0].size();
    uint16_t height = sandpile.size();
    sandpile_copy = sandpile;
    bool isCollapsePossible = false;
    bool upperLine = false;
    bool lowerLine = false;
    bool leftColumn = false;
    bool rightColumn = false;

    for (uint16_t i = 0; i < height; i++) {
        for (uint16_t j = 0; j < width; j++) {
            if (sandpile_copy[i][j] > 3) {
                isCollapsePossible = true;
                if ((i + 1 >= height) && !lowerLine) {
                    AddLowerLine(sandpile);
                    lowerLine = true;
                }
                if ((i - 1 < 0) && !upperLine) {
                    AddUpperLine(sandpile);
                    upperLine = true;
                }
                if ((j + 1 >= width) && !rightColumn) {
                    AddRightColumn(sandpile);
                    rightColumn = true;
                }
                if ((j - 1 < 0) && !leftColumn) {
                    AddLeftColumn(sandpile);
                    leftColumn = true;
                }
                SandMove(sandpile, i, j, upperLine, leftColumn);
            }
        }
    }

    return isCollapsePossible;
}

void SetLengthWidth(std::deque<std::deque<uint64_t>>& sandpile, uint16_t length, uint16_t width) {
    std::deque<uint64_t> line;
    line.resize(width);

    for (size_t i = 0; i < length; i++) {
        sandpile.push_back(line);
    }
}

void SetValues(std::deque<std::deque<uint64_t>>& sandpile, const std::string& filename) {
    std::ifstream file;
    file.open(filename);
    uint16_t x, y;
    uint64_t value;

    while (!file.eof()) {
        file >> x;
        if (file.eof()) {
            break;
        }
        file >> y >> value;
        sandpile[x][y] = value;
    }
}

void SandCollapseCycle(std::deque<std::deque<uint64_t>>& sandpile, uint64_t max_iter, uint64_t freq,
                       const std::string& path) {
    uint64_t iter = 0;
    bool onlyLastCondition = freq == 0;
    std::string full_path = path + "\\";
    std::string ext = ".bmp";
    std::deque<std::deque<uint64_t>> sandpile_copy = sandpile;

    if (!onlyLastCondition) {
        ToImage(sandpile, full_path + std::to_string(iter / freq) + ext);
    }

    while ((iter < max_iter) && SandCollapse(sandpile, sandpile_copy)) {
        iter++;
        if ((!onlyLastCondition) && (iter % freq == 0)) {
            ToImage(sandpile, full_path + std::to_string(iter / freq) + ext);
        }
    }

    if (onlyLastCondition) {
        ToImage(sandpile, full_path + "0" + ext);
    }
}