#include <iterator>

namespace MySTL {

    template<typename Iterator>
    concept InputIterator = requires {
        requires std::input_iterator<Iterator>;
        requires std::is_copy_constructible_v<std::iter_value_t<Iterator>>;
    };

    template<typename Iterator>
    concept ForwardIterator = requires {
        requires std::forward_iterator<Iterator>;
        requires std::is_copy_constructible_v<std::iter_value_t<Iterator>>;
    };

    template<typename Iterator>
    concept BidirectionalIterator = requires {
        requires std::bidirectional_iterator<Iterator>;
        requires std::is_copy_constructible_v<std::iter_value_t<Iterator>>;
    };

    template<typename Predicate, typename Iterator>
    concept UnaryPredicate = std::predicate<Predicate, std::iter_value_t<Iterator>>;

    template<typename Predicate, typename Iterator>
    concept BinaryPredicate = std::predicate<Predicate, std::iter_value_t<Iterator>, std::iter_value_t<Iterator>>;

    template<typename Iterator, typename Predicate>
    requires InputIterator<Iterator> && UnaryPredicate<Predicate, Iterator>
    constexpr bool all_of(Iterator first, Iterator last, Predicate predicate) {
        while (first != last) {
            if (!predicate(*first)) {
                return false;
            }
            ++first;
        }
        return true;
    }

    template<typename Iterator, typename Predicate>
    requires InputIterator<Iterator> && UnaryPredicate<Predicate, Iterator>
    constexpr bool any_of(Iterator first, Iterator last, Predicate predicate) {
        while (first != last) {
            if (predicate(*first)) {
                return true;
            }
            ++first;
        }
        return false;
    }

    template<typename Iterator, typename Predicate>
    requires InputIterator<Iterator> && UnaryPredicate<Predicate, Iterator>
    constexpr bool none_of(Iterator first, Iterator last, Predicate predicate) {
        while (first != last) {
            if (predicate(*first)) {
                return false;
            }
            ++first;
        }
        return true;
    }

    template<typename Iterator, typename Predicate>
    requires InputIterator<Iterator> && UnaryPredicate<Predicate, Iterator>
    constexpr bool one_of(Iterator first, Iterator last, Predicate predicate) {
        bool is_one = false;
        while (first != last) {
            if (predicate(*first) && is_one) {
                return false;
            } else if (predicate(*first) && !is_one) {
                is_one = true;
            }
            ++first;
        }
        return is_one;
    }

    template<typename Iterator, typename Predicate>
    requires ForwardIterator<Iterator> && BinaryPredicate<Predicate, Iterator>
    constexpr bool is_sorted(Iterator first, Iterator last, Predicate comparator) {
        if (first == last) {
            return true;
        }
        auto next = first;
        ++next;
        while (next != last) {
            if (!comparator(*(first++), *(next++)) || comparator(*(next++), *(first++))) {
                return false;
            }
        }
        return true;
    }

    template<typename Iterator, typename Predicate>
    requires InputIterator<Iterator> && UnaryPredicate<Predicate, Iterator>
    constexpr bool is_partitioned(Iterator first, Iterator last, Predicate predicate) {
        if (first == last) {
            return true;
        }
        while (first != last) {
            if (!predicate(*first)) {
                break;
            }
            ++first;
        }
        while (first != last) {
            if (predicate(*first)) {
                return false;
            }
            ++first;
        }
        return true;
    }

    template<typename Iterator>
    requires InputIterator<Iterator>
    constexpr Iterator find_not(Iterator first, Iterator last, const std::iter_value_t<Iterator>& value) {
        while (first != last) {
            if (*first != value) {
                return first;
            }
            ++first;
        }
        return last;
    }

    template<typename Iterator>
    requires BidirectionalIterator<Iterator>
    constexpr Iterator find_backward(Iterator first, Iterator last, const std::iter_value_t<Iterator>& value) {
        for (auto iter = last; iter != first;) {
            iter--;
            if (*iter == value) {
                return iter;
            }
        }
        return last;
    }

    template<typename Iterator, typename Predicate>
    requires BidirectionalIterator<Iterator> && BinaryPredicate<Predicate, Iterator>
    constexpr bool is_palindrome(Iterator first, Iterator last, Predicate comparator) {
        if (first == last || first == --last) {
            return true;
        }
        while (first != last) {
            if (!comparator(*first, *last)) {
                return false;
            }
            if (++first == last) {
                break;
            }
            if (first == --last) {
                break;
            }
        }
        return true;
    }

}