#include "image.h"
#include <deque>
#include <string>

void SetLengthWidth(std::deque<std::deque<uint64_t>>& sandpile, uint16_t length, uint16_t width);

void SetValues(std::deque<std::deque<uint64_t>>& sandpile, const std::string& filename);

void SandCollapseCycle(std::deque<std::deque<uint64_t>>& sandpile, uint64_t max_iter, uint64_t freq,
                       const std::string& path);