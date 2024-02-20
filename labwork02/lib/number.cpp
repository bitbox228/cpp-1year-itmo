#include "number.h"


uint2022_t from_uint(uint32_t i) {
    uint2022_t value;
    uint32_t start_value;
    start_value = i;
    value.digits[0] = start_value % uint2022_t::DIGIT_SIZE;
    if ((start_value / uint2022_t::DIGIT_SIZE) != 0) {
        value.digits[1] = start_value / uint2022_t::DIGIT_SIZE;
        value.current_size = 2;
    } else {
        value.current_size = 1;
    }
    return value;
}

uint2022_t from_string(const char* buff) {
    uint2022_t value;
    std::string str = buff;
    for (size_t i = str.length(); i > 0; i -= std::min(i, uint2022_t::DIGIT_LEN)) {
        value.digits[value.current_size++] = std::stoul(
                str.substr(i - std::min(i, uint2022_t::DIGIT_LEN), std::min(i, uint2022_t::DIGIT_LEN)));
    }
    return value;
}

void uint2022_t::digit_shift() {
    current_size++;
    for (size_t j = current_size - 1; j > 0; j--) {
        digits[j] = digits[j - 1];
    }
    digits[0] = 0;
}

void uint2022_t::digits_clear() {
    for (size_t i = 0; i < current_size; i++) {
        digits[i] = 0;
    }
    current_size = 0;
}

void uint2022_t::remove_zeros() {
    if (current_size == 0) {
        return;
    }
    size_t i = current_size - 1;
    while (current_size > 1 && digits[i] == 0) {
        current_size--;
        i--;
    }
}

uint2022_t operator+(const uint2022_t& lhs, const uint2022_t& rhs) {
    uint2022_t value;
    size_t left_size = lhs.current_size;
    size_t right_size = rhs.current_size;
    uint32_t next_digit_more = 0;
    size_t total_len = std::max(left_size, right_size);
    bool is_right_bigger;
    if (total_len == left_size) {
        is_right_bigger = false;
    } else {
        is_right_bigger = true;
    }
    for (size_t i = 0; (i < total_len) || (next_digit_more != 0); i++) {
        if ((i < left_size) && (i < right_size)) {
            value.digits[i] = lhs.digits[i] + rhs.digits[i] + next_digit_more;
            next_digit_more = value.digits[i] / uint2022_t::DIGIT_SIZE;
            value.digits[i] %= uint2022_t::DIGIT_SIZE;
        } else {
            if (is_right_bigger) {
                value.digits[i] = rhs.digits[i] + next_digit_more;
            } else {
                value.digits[i] = lhs.digits[i] + next_digit_more;
            }
            next_digit_more = value.digits[i] / uint2022_t::DIGIT_SIZE;
        }
        value.current_size++;
    }
    return value;
}

uint2022_t operator-(const uint2022_t& lhs, const uint2022_t& rhs) {
    uint2022_t value;
    if (lhs == rhs) {
        return from_string("0");
    }
    uint32_t next_digit_less = 0;
    if (lhs > rhs) {
        for (size_t i = 0; i < rhs.current_size; i++) {
            if (lhs.digits[i] - next_digit_less < rhs.digits[i]) {
                value.digits[i] = lhs.digits[i] + uint2022_t::DIGIT_SIZE - next_digit_less - rhs.digits[i];
                next_digit_less = 1;
            } else {
                value.digits[i] = lhs.digits[i] - next_digit_less - rhs.digits[i];
                next_digit_less = 0;
            }
            value.current_size++;
        }
        for (size_t i = rhs.current_size; i < lhs.current_size; i++) {
            value.digits[i] = lhs.digits[i] - next_digit_less;
            next_digit_less = 0;
            value.current_size++;
        }
    }
    return value;
}

uint2022_t operator*(const uint2022_t& lhs, const uint2022_t& rhs) {
    uint2022_t value;
    uint2022_t digit_multi;
    uint32_t temp_int;
    size_t left_size = lhs.current_size;
    size_t right_size = rhs.current_size;
    uint64_t temp_long;
    uint64_t next_digit_more = 0;
    if ((left_size == 1 && lhs.digits[0] == 0) || (right_size == 1 && rhs.digits[0] == 0)) {
        value = from_uint(0);
    } else {
        for (int i = 0; i < right_size; i++) {
            digit_multi.digits_clear();
            digit_multi.current_size = i;
            for (int j = 0; j < left_size; j++) {
                temp_long = (uint64_t{lhs.digits[j]}) * (uint64_t{rhs.digits[i]}) + next_digit_more;
                next_digit_more = temp_long / uint2022_t::DIGIT_SIZE;
                temp_int = temp_long % uint2022_t::DIGIT_SIZE;
                digit_multi.digits[digit_multi.current_size] = temp_int;
                digit_multi.current_size++;
            }
            if (next_digit_more != 0) {
                digit_multi.digits[digit_multi.current_size] = {static_cast<uint32_t>(next_digit_more)};
                digit_multi.current_size++;
            }
            next_digit_more = 0;
            value = value + digit_multi;
        }
    }
    return value;
}

uint2022_t operator/(const uint2022_t& lhs, const uint2022_t& rhs) {
    uint2022_t result_value;
    uint2022_t current_value;
    if (rhs == from_uint(0)) {
        return result_value;
    }
    for (size_t i = lhs.current_size; i-- > 0;) {
        current_value.digit_shift();
        current_value.digits[0] = lhs.digits[i];
        current_value.remove_zeros();
        if (current_value >= rhs) {
            uint32_t digit = 0;
            uint32_t left = 0;
            uint32_t right = uint2022_t::DIGIT_SIZE;
            while (left <= right) {
                uint32_t middle = (left + right) / 2;
                uint2022_t t = rhs * from_uint(middle);
                if (t <= current_value) {
                    digit = middle;
                    left = middle + 1;
                } else {
                    right = middle - 1;
                }
            }
            result_value.digit_shift();
            result_value.digits[0] = digit;
            current_value = current_value - rhs * from_uint(digit);
        }
    }
    return result_value;
}

bool operator==(const uint2022_t& lhs, const uint2022_t& rhs) {
    if (lhs.current_size != rhs.current_size) {
        return false;
    }
    for (size_t i = 0; i < lhs.current_size; i++) {
        if (lhs.digits[i] != rhs.digits[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(const uint2022_t& lhs, const uint2022_t& rhs) {
    return !(lhs == rhs);
}

bool operator>=(const uint2022_t& lhs, const uint2022_t& rhs) {
    bool is_left_bigger = true;
    if ((lhs != rhs) && (lhs.current_size == rhs.current_size)) {
        for (size_t i = lhs.current_size; i-- > 0;) {
            if (lhs.digits[i] > rhs.digits[i]) {
                break;
            } else if (lhs.digits[i] < rhs.digits[i]) {
                is_left_bigger = false;
                break;
            }
        }
    } else if ((lhs != rhs) && (lhs.current_size < rhs.current_size)) {
        is_left_bigger = false;
    }
    return is_left_bigger;
}

bool operator<(const uint2022_t& lhs, const uint2022_t& rhs) {
    return !(lhs >= rhs);
}

bool operator>(const uint2022_t& lhs, const uint2022_t& rhs) {
    return (rhs < lhs);
}

bool operator<=(const uint2022_t& lhs, const uint2022_t& rhs) {
    return (rhs >= lhs);
}

std::ostream& operator<<(std::ostream& stream, const uint2022_t& value) {
    char zero_fill = stream.fill('0');
    if (value.current_size == 0) {
        stream << 0;
    } else {
        stream << value.digits[value.current_size - 1];
        for (size_t i = value.current_size - 1; i-- > 0;) {
            stream << std::setw(9) << value.digits[i];
        }
        stream.fill(zero_fill);
    }
    return stream;
}