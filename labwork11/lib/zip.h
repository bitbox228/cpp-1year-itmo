#include <iterator>
#include <ranges>

template<typename T, typename S> requires std::ranges::forward_range<T> && std::ranges::forward_range<S>
class zip {
public:
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<typename T::value_type, typename S::value_type>;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;

    private:

        T::iterator first_iterator_;
        S::iterator second_iterator_;

    public:
        Iterator(T::iterator first_iterator, S::iterator second_iterator) : first_iterator_(first_iterator),
                                                                            second_iterator_(second_iterator) {
        }

        value_type operator*() const {
            return std::make_pair(*first_iterator_, *second_iterator_);
        }

        Iterator& operator++() {
            ++first_iterator_;
            ++second_iterator_;
            return *this;
        }

        Iterator operator++(int) {
            auto tmp = *this;
            operator++();
            return tmp;
        }

        bool operator==(const Iterator& rhs) const {
            return first_iterator_ == rhs.first_iterator_ || second_iterator_ == rhs.second_iterator_;
        }

        bool operator!=(const Iterator& rhs) const {
            return !(*this == rhs);
        }
    };

    using value_type = std::pair<typename T::value_type, typename S::value_type>;
    using iterator = Iterator;

private:

    T::iterator first_container_begin_;
    T::iterator first_container_end_;
    S::iterator second_container_begin_;
    S::iterator second_container_end_;

public:

    zip(T& first_container, S& second_container) : first_container_begin_(std::begin(first_container)),
                                                               first_container_end_(std::end(first_container)),
                                                               second_container_begin_(std::begin(second_container)),
                                                               second_container_end_(std::end(second_container)) {
    }

    iterator begin() const {
        return iterator(first_container_begin_, second_container_begin_);
    }

    iterator end() const {
        return iterator(first_container_end_, second_container_end_);
    }
};
