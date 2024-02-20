#include <iterator>
#include <cassert>

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template<Arithmetic T>
class xrange {
public:

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using reference = T&;
        using difference_type = T;
        using const_reference = const T&;
        using pointer = T*;

    private:
        const xrange& xrange_;
        T value_;

    public:
        Iterator(value_type value, const xrange& xrange) : value_(value), xrange_(xrange) {
        }

        value_type operator*() const {
            return value_;
        }

        Iterator& operator++() {
            value_ += xrange_.step_;
            return *this;
        }

        Iterator operator++(int) {
            auto tmp = *this;
            operator++();
            return tmp;
        }

        bool operator==(const Iterator& rhs) const {
            if (xrange_ != rhs.xrange_) {
                return false;
            }
            if (value_ == rhs.value_ ||
                xrange_.step_ > static_cast<T>(0) && value_ >= xrange_.end_ && rhs.value_ >= rhs.xrange_.end_ &&
                value_ - xrange_.step_ < xrange_.end_ &&
                rhs.value_ - rhs.xrange_.step_ < rhs.xrange_.end_ ||
                xrange_.step_ < static_cast<T>(0) && value_ <= xrange_.end_ && rhs.value_ <= rhs.xrange_.end_ &&
                value_ - xrange_.step_ > xrange_.end_ &&
                rhs.value_ - rhs.xrange_.step_ > rhs.xrange_.end_) {
                return true;
            }
            return false;
        }

        bool operator!=(const Iterator& rhs) const {
            return !(*this == rhs);
        }
    };

    using value_type = T;
    using iterator = Iterator;

private:
    value_type begin_;
    value_type end_;
    value_type step_;

public:
    xrange(value_type begin, value_type end, value_type step = static_cast<value_type>(1)) : begin_(begin), end_(end), step_(step) {
        assert((begin < end && step > 0) || (begin > end && step < 0));
    }

    xrange(value_type end) : begin_(static_cast<value_type>(0)), end_(end), step_(static_cast<value_type>(1)) {
    }

    iterator begin() const {
        return iterator(begin_, *this);
    }

    iterator end() const {
        return iterator(end_, *this);
    }

    bool operator==(const xrange& rhs) const {
        return begin_ == rhs.begin_ && end_ == rhs.end_ && step_ == rhs.step_;
    }

    bool operator!=(const xrange& rhs) const {
        return !(*this == rhs);
    }
};