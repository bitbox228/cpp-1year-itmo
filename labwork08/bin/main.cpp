#include <iostream>
#include <algorithm>
#include "../lib/CCircularBuffer.h"
#include <vector>

template<typename T>
void print(const std::vector<T>& a) {
    for (size_t i = 0; i < a.size(); i++) {
        std::cout << a[i] << ' ';
    }
    std::cout << '\n';
}

int main() {
    CCircularBuffer<int> buff{1, 100, 21002, -5, -153, 12030};
    std::sort(buff.begin(), buff.end());

    std::cout << buff;
}