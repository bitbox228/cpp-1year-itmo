#pragma once

#include <iterator>
#include <type_traits>
#include <memory>
#include <stdexcept>

template<typename T>

class Iterator : public std::iterator<std::random_access_iterator_tag, T> {
public:
    using value_type = T;
    using reference = T&;
    using pointer = T*;
    using const_reference = const T&;
    using difference_type = int64_t;
    using size_type = size_t;
private:
    pointer cur_elem_;
    pointer begin_;
    pointer end_;
    size_type size_;
public:

    pointer cur_elem() const {
        return cur_elem_;
    }

    pointer begin() const {
        return begin_;
    }

    pointer end() const {
        return end_;
    }

    size_type size() const {
        return size_;
    }

    Iterator() = default;

    Iterator(pointer buff) : cur_elem_(buff), begin_(buff), end_(buff), size_(0) {}

    Iterator(pointer buff, size_type size, pointer cur_elem = nullptr) : size_(size), cur_elem_(cur_elem) {
        begin_ = buff;
        end_ = buff + size_ - 1;
        if (cur_elem_ == nullptr) {
            cur_elem_ = begin_;
        }
    }

    Iterator(const Iterator<T>& other) {
        cur_elem_ = other.cur_elem_;
        begin_ = other.begin_;
        end_ = other.end_;
        size_ = other.size_;
    }

    Iterator& operator=(const Iterator<T>& rhs) {
        if (this == &rhs) {
            return *this;
        }
        size_ = rhs.size_;
        begin_ = rhs.begin_;
        end_ = rhs.end_;
        cur_elem_ = rhs.cur_elem_;
        return *this;
    }

    Iterator& operator+=(difference_type n) {
        n %= size_;
        if (n < 0) {
            n += size_;
        }
        size_t offset_to_end = end_ - cur_elem_;
        if (offset_to_end >= n) {
            cur_elem_ += n;
        } else {
            cur_elem_ = begin_ + (n - offset_to_end - 1);
        }
        return *this;
    }

    Iterator& operator++() {
        return *this += 1;
    }

    Iterator operator++(int) {
        auto temp = *this;
        operator++();
        return temp;
    }

    Iterator operator+(difference_type rhs) const {
        Iterator<T> temp = *this;
        return temp += rhs;
    }

    Iterator& operator-=(difference_type n) {
        return *this += -n;
    }

    Iterator& operator--() {
        return *this -= 1;
    }

    Iterator operator--(int) {
        auto temp = *this;
        operator--();
        return temp;
    }

    Iterator operator-(difference_type rhs) const {
        Iterator<T> temp = *this;
        return temp -= rhs;
    }

    difference_type operator-(const Iterator<T>& rhs) const {
        return (this->cur_elem_ - rhs.cur_elem_);
    }

    reference operator*() const {
        return *cur_elem_;
    }

    reference operator[](size_t n) const {
        return *(*this + n);
    }

    bool operator<(const Iterator<T>& rhs) const {
        return rhs - *this > 0;
    }

    bool operator>(const Iterator<T>& rhs) const {
        return rhs < *this;
    }

    bool operator>=(const Iterator<T>& rhs) const {
        return !(*this < rhs);
    }

    bool operator<=(const Iterator<T>& rhs) const {
        return !(*this > rhs);
    }

    bool operator==(const Iterator<T>& rhs) const {
        return (this->cur_elem_ == rhs.cur_elem_);
    }

    bool operator!=(const Iterator<T>& rhs) const {
        return !(*this == rhs);
    }

    Iterator<const T> ToConst() const {
        return Iterator<const T>(begin_, size_, cur_elem_);
    }
};

template<typename T, typename Allocator = std::allocator<T>>

class CCircularBuffer {
public:
    using allocator_type = Allocator;
    using value_type = T;
    using reference = T&;
    using pointer = T*;
    using const_reference = const T&;
    using difference_type = int64_t;
    using size_type = size_t;

    using iterator = Iterator<T>;
    using const_iterator = Iterator<const T>;

private:

    static const size_type kMinCapacity = 1;

    pointer buff_;
    allocator_type alloc_;
    size_type size_;
    size_type capacity_;
    iterator begin_;
    iterator end_;

public:
    CCircularBuffer() : capacity_(kMinCapacity), size_(0) {
        buff_ = alloc_.allocate(capacity_);
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_);
    }

    ~CCircularBuffer() {
        this->clear();
        alloc_.deallocate(buff_, capacity_);
    }

    template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>>
    CCircularBuffer(InputIterator i, InputIterator j) {
        capacity_ = j - i + 1;
        size_ = capacity_ - 1;
        buff_ = alloc_.allocate(capacity_);
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
        auto this_it = begin_;
        auto other_it = i;
        while (other_it != j) {
            std::construct_at(&*this_it++, *other_it++);
        }
    }

    CCircularBuffer(const std::initializer_list<T>& l) : capacity_(l.size() + 1), size_(l.size()) {
        buff_ = alloc_.allocate(capacity_);
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
        auto list_it = l.begin();
        auto it = begin_;
        while (list_it != l.end()) {
            std::construct_at(&*it++, *list_it++);
        }
    }

    CCircularBuffer(size_type n, value_type value = value_type()) : capacity_(n + 1), size_(n) {
        buff_ = alloc_.allocate(capacity_);
        for (size_t i = 0; i < n; i++) {
            std::construct_at(&buff_[i], value);
        }
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
    }

    CCircularBuffer(const CCircularBuffer& other) : size_(other.size_), capacity_(other.capacity_) {
        buff_ = alloc_.allocate(capacity_);
        begin_ = Iterator(buff_, capacity_);
        end_ = Iterator(buff_, capacity_) + size_;
        auto this_it = this->begin_;
        auto other_it = other.begin_;
        for (size_t i = 0; i < size_; i++, this_it++, other_it++) {
            std::construct_at(&*this_it, *other_it);
        }
    }

    iterator begin() const {
        return begin_;
    }

    iterator end() const {
        return end_;
    }

    const_iterator cbegin() const {
        return begin_.ToConst();
    }

    const_iterator cend() const {
        return end_.ToConst();
    }

    size_type size() const {
        return size_;
    }

    size_type max_size() const {
        return std::allocator_traits<Allocator>::max_size(alloc_);
    }

    size_type capacity() const {
        return capacity_ - 1;
    }

    bool empty() const {
        return (size_ == 0);
    }

    CCircularBuffer& operator=(const CCircularBuffer& other) {
        if (*this == other) {
            return *this;
        }
        this->clear();
        alloc_.deallocate(buff_, capacity_);
        capacity_ = other.capacity_;
        size_ = other.size_;
        buff_ = alloc_.allocate(capacity_);
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
        auto this_it = begin_;
        auto other_it = other.begin_;
        for (size_t i = 0; i < size_; i++, this_it++, other_it++) {
            std::construct_at(&*this_it, *other_it);
        }
        return *this;
    }

    CCircularBuffer& operator=(const std::initializer_list<T>& il) {
        this->clear();
        alloc_.deallocate(buff_, capacity_);
        capacity_ = il.size() + 1;
        size_ = il.size();
        buff_ = alloc_.allocate(capacity_);
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
        auto list_it = il.begin();
        auto it = begin_;
        while (list_it != il.end()) {
            std::construct_at(&*it++, *list_it++);
        }
        return *this;
    }

    bool operator==(const CCircularBuffer& other) {
        if (size_ != other.size_) {
            return false;
        }
        auto this_it = begin_;
        auto other_it = other.begin_;
        for (size_t i = 0; i < size_; i++, this_it++, other_it++) {
            if (*this_it != *other_it) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const CCircularBuffer& other) {
        return !(*this == other);
    }

    const_reference operator[](size_type index) const {
        return begin_[index];
    }

    reference operator[](size_type index) {
        return begin_[index];
    }

    void swap(CCircularBuffer& other) {
        std::swap(buff_, other.buff_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
    }

    static void swap(CCircularBuffer& lhs, CCircularBuffer& rhs) {
        std::swap(lhs.buff_, rhs.buff_);
        std::swap(lhs.size_, rhs.size_);
        std::swap(lhs.capacity_, rhs.capacity_);
        std::swap(lhs.begin_, rhs.begin_);
        std::swap(lhs.end_, rhs.end_);
    }

    void clear() {
        std::destroy_n(buff_, size_);
        size_ = 0;
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_);
    }

    void resize(size_type n, const_reference value = value_type()) {
        if (capacity_ == n) {
            while (begin_ != end_ + 1) {
                *end_++ = value;
            }
            size_ = capacity_;
            return;
        }
        pointer new_buff = alloc_.allocate(n + 1);
        for (size_t i = 0; i < n; i++) {
            if (i < size_) {
                std::construct_at(new_buff + i, begin_[i]);
            } else {
                std::construct_at(new_buff + i, value);
            }
        }
        this->clear();
        alloc_.deallocate(buff_, capacity_);
        buff_ = new_buff;
        capacity_ = n + 1;
        size_ = n;
        begin_ = iterator(new_buff, capacity_);
        end_ = iterator(new_buff, capacity_) + size_;
    }

    reference front() {
        if (empty()) {
            throw std::out_of_range("Tried to get an element from an empty buffer");
        }
        return *(this->begin());
    }

    const_reference front() const {
        if (empty()) {
            throw std::out_of_range("Tried to get an element from an empty buffer");
        }
        return *(this->begin());
    }

    reference back() {
        if (empty()) {
            throw std::out_of_range("Tried to get an element from an empty buffer");
        }
        auto tmp = this->end();
        --tmp;
        return *tmp;
    }

    const_reference back() const {
        if (empty()) {
            throw std::out_of_range("Tried to get an element from an empty buffer");
        }
        auto tmp = this->end();
        --tmp;
        return *tmp;
    }

    void push_back(const_reference value) {
        if (size_ < capacity_ - 1) {
            std::construct_at(&*end_, value);
            end_++;
            size_++;
        } else {
            std::construct_at(&*end_, value);
            end_++;
            begin_++;
            std::destroy_at(&*end_);
        }
    }

    void push_front(const_reference value) {
        if (size_ < capacity_ - 1) {
            begin_--;
            std::construct_at(&*begin_, value);
            size_++;
        } else {
            begin_--;
            end_--;
            std::destroy_at(&*end_);
            std::construct_at(&*begin_, value);
        }
    }

    void pop_back() {
        if (empty()) {
            throw std::out_of_range("Tried to pop from an empty buffer");
        }
        end_--;
        std::destroy_at(&*end_);
        size_--;
    }

    void pop_front() {
        if (empty()) {
            throw std::out_of_range("Tried to pop from an empty buffer");
        }
        std::destroy_at(&*begin_);
        begin_++;
        size_--;
    }

    iterator erase(const_iterator q) {
        if (empty()) {
            throw std::out_of_range("Tried to erase from an empty buffer");
        }
        iterator it(buff_, capacity_, const_cast<T*>(q.cur_elem()));
        while (it + 1 != end_) {
            std::destroy_at(&*it);
            std::construct_at(&*it, *(it + 1));
            it++;
        }
        std::destroy_at(&*it);
        size_--;
        end_--;
        return iterator(buff_, capacity_, const_cast<T*>(q.cur_elem()));
    }

    iterator erase(const_iterator q1, const_iterator q2) {
        if (empty()) {
            throw std::out_of_range("Tried to erase from an empty buffer");
        }
        size_type len;
        if (q1 < q2) {
            len = q2 - q1;
        } else {
            len = capacity_ - (q1 - q2);
        }
        if (len > size_) {
            this->clear();
            return end_;
        }
        auto q = q1;
        while (q != q2) {
            std::destroy_at(&*q);
            q++;
        }
        iterator it(buff_, capacity_, const_cast<T*>(q2.cur_elem()));
        while (it != end_) {
            std::construct_at(&*(it - len), *it);
            std::destroy_at(&*it);
            it++;
        }
        size_ -= len;
        end_ -= len;
        iterator result(buff_, capacity_, const_cast<T*>(q2.cur_elem()));
        if ((end_ < begin_ && result >= end_ && result < begin_) ||
            (end_ > begin_ && (result >= end_ || result < begin_)) || empty()) {
            return end_;
        }
        return result;
    }

    template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>>
    void assign(InputIterator i, InputIterator j) {
        this->clear();
        alloc_.deallocate(buff_, capacity_);
        capacity_ = j - i + 1;
        size_ = capacity_ - 1;
        buff_ = alloc_.allocate(capacity_);
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
        auto this_it = begin_;
        auto other_it = i;
        while (other_it != j) {
            std::construct_at(&*this_it++, *other_it++);
        }
    }

    void assign(const std::initializer_list<value_type>& il) {
        assign(il.begin(), il.end());
    }

    void assign(size_type n, value_type t) {
        this->clear();
        alloc_.deallocate(buff_, capacity_);
        capacity_ = n + 1;
        size_ = capacity_ - 1;
        buff_ = alloc_.allocate(capacity_);
        for (size_t i = 0; i < n; i++) {
            std::construct_at(&buff_[i], t);
        }
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
    }

    reference at(size_type index) {
        if (empty()) {
            throw std::out_of_range("Tried to get an element from an empty buffer");
        }
        return begin_(index);
    }

    const_reference at(size_type index) const {
        if (empty()) {
            throw std::out_of_range("Tried to get an element from an empty buffer");
        }
        return begin_(index);
    }

    void reserve(size_type n) {
        if (n <= capacity_ - 1) {
            return;
        }
        auto old_buff = buff_;
        auto it = begin_;
        auto it_end = end_;
        auto old_cap = capacity_;
        auto old_size = size_;
        capacity_ = n + 1;
        buff_ = alloc_.allocate(capacity_);
        size_ = 0;
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_);
        while (it != it_end) {
            push_back(*it);
            it++;
        }
        std::destroy_n(old_buff, old_size);
        alloc_.deallocate(old_buff, old_cap);
    }

    iterator insert(const_iterator p, size_type n, value_type t) {
        if (n == 0) {
            return iterator(buff_, capacity_, const_cast<T*>(p.cur_elem()));
        }
        iterator result;
        auto old_buff = buff_;
        auto old_size = size_;
        auto it = begin_;
        auto it_end = end_;
        iterator it_p(buff_, capacity_, const_cast<T*>(p.cur_elem()));
        buff_ = alloc_.allocate(capacity_);
        size_ = 0;
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_);
        while (it != it_end) {
            push_back(*it);
            if (it + 1 == it_p) {
                result = end_;
                for (size_t i = 0; i < n; i++) {
                    push_back(t);
                }
            }
            it++;
        }
        std::destroy_n(old_buff, old_size);
        alloc_.deallocate(old_buff, capacity_);
        return result;
    }

    iterator insert(const_iterator p, value_type t) {
        return insert(p, 1, t);
    }

    template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>>
    void insert(const_iterator p, InputIterator i, InputIterator j) {
        if (i == j) {
            return iterator(buff_, capacity_, const_cast<T*>(p.cur_elem()));
        }
        iterator result;
        auto old_buff = buff_;
        auto old_size = size_;
        auto it = begin_;
        auto it_end = end_;
        iterator it_p(buff_, capacity_, const_cast<T*>(p.cur_elem()));
        buff_ = alloc_.allocate(capacity_);
        size_ = 0;
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_);
        while (it != it_end) {
            push_back(*it);
            if (it + 1 == it_p) {
                result = end_;
                while (i != j) {
                    push_back(*i);
                    i++;
                }
            }
            it++;
        }
        std::destroy_n(old_buff, old_size);
        alloc_.deallocate(old_buff, capacity_);
        return result;
    }

    iterator insert(const_iterator p, const std::initializer_list<T>& il) {
        return insert(p, il.begin(), il.end());
    }
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const CCircularBuffer<T>& a) {
    auto it = a.begin();
    for (size_t i = 0; i < a.size(); i++, it++) {
        os << *it << ' ';
    }
    return os;
}

template<typename T, typename Allocator = std::allocator<T>>

class CCircularBufferExt {
public:
    using allocator_type = Allocator;
    using value_type = T;
    using reference = T&;
    using pointer = T*;
    using const_reference = const T&;
    using difference_type = int64_t;
    using size_type = size_t;

    using iterator = Iterator<T>;
    using const_iterator = Iterator<const T>;

private:

    static const size_type kMinCapacity = 3;
    static const size_type kCapacityCoefficient = 2;

    pointer buff_;
    allocator_type alloc_;
    size_type size_;
    size_type capacity_;
    iterator begin_;
    iterator end_;

public:
    CCircularBufferExt() : capacity_(kMinCapacity), size_(0) {
        buff_ = alloc_.allocate(capacity_);
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_);
    }

    ~CCircularBufferExt() {
        this->clear();
        alloc_.deallocate(buff_, capacity_);
    }

    template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>>
    CCircularBufferExt(InputIterator i, InputIterator j) {
        capacity_ = j - i + 1;
        size_ = capacity_ - 1;
        buff_ = alloc_.allocate(capacity_);
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
        auto this_it = begin_;
        auto other_it = i;
        while (other_it != j) {
            std::construct_at(&*this_it++, *other_it++);
        }
    }

    CCircularBufferExt(const std::initializer_list<T>& l) : capacity_(l.size() + 1), size_(l.size()) {
        buff_ = alloc_.allocate(capacity_);
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
        auto list_it = l.begin();
        auto it = begin_;
        while (list_it != l.end()) {
            std::construct_at(&*it++, *list_it++);
        }
    }

    CCircularBufferExt(size_type n, value_type value = value_type()) : capacity_(n + 1), size_(n) {
        buff_ = alloc_.allocate(capacity_);
        for (size_t i = 0; i < n; i++) {
            std::construct_at(&buff_[i], value);
        }
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
    }

    CCircularBufferExt(const CCircularBufferExt& other) : size_(other.size_), capacity_(other.capacity_) {
        buff_ = alloc_.allocate(capacity_);
        begin_ = Iterator(buff_, capacity_);
        end_ = Iterator(buff_, capacity_) + size_;
        auto this_it = this->begin_;
        auto other_it = other.begin_;
        for (size_t i = 0; i < size_; i++, this_it++, other_it++) {
            std::construct_at(&*this_it, *other_it);
        }
    }

    iterator begin() const {
        return begin_;
    }

    iterator end() const {
        return end_;
    }

    const_iterator cbegin() const {
        return begin_.ToConst();
    }

    const_iterator cend() const {
        return end_.ToConst();
    }

    size_type size() const {
        return size_;
    }

    size_type max_size() const {
        return std::allocator_traits<Allocator>::max_size(alloc_);
    }

    size_type capacity() const {
        return capacity_ - 1;
    }

    bool empty() const {
        return (size_ == 0);
    }

    CCircularBufferExt& operator=(const CCircularBufferExt& other) {
        if (*this == other) {
            return *this;
        }
        this->clear();
        alloc_.deallocate(buff_, capacity_);
        capacity_ = other.capacity_;
        size_ = other.size_;
        buff_ = alloc_.allocate(capacity_);
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
        auto this_it = begin_;
        auto other_it = other.begin_;
        for (size_t i = 0; i < size_; i++, this_it++, other_it++) {
            std::construct_at(&*this_it, *other_it);
        }
        return *this;
    }

    CCircularBufferExt& operator=(const std::initializer_list<T>& il) {
        this->clear();
        alloc_.deallocate(buff_, capacity_);
        capacity_ = il.size() + 1;
        size_ = il.size();
        buff_ = alloc_.allocate(capacity_);
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
        auto list_it = il.begin();
        auto it = begin_;
        while (list_it != il.end()) {
            std::construct_at(&*it++, *list_it++);
        }
        return *this;
    }

    bool operator==(const CCircularBufferExt& other) {
        if (size_ != other.size_) {
            return false;
        }
        auto this_it = begin_;
        auto other_it = other.begin_;
        for (size_t i = 0; i < size_; i++, this_it++, other_it++) {
            if (*this_it != *other_it) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const CCircularBufferExt& other) {
        return !(*this == other);
    }

    const_reference operator[](size_type index) const {
        return begin_[index];
    }

    reference operator[](size_type index) {
        return begin_[index];
    }

    void swap(CCircularBufferExt& other) {
        std::swap(buff_, other.buff_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
    }

    static void swap(CCircularBufferExt& lhs, CCircularBufferExt& rhs) {
        std::swap(lhs.buff_, rhs.buff_);
        std::swap(lhs.size_, rhs.size_);
        std::swap(lhs.capacity_, rhs.capacity_);
        std::swap(lhs.begin_, rhs.begin_);
        std::swap(lhs.end_, rhs.end_);
    }

    void clear() {
        std::destroy_n(buff_, size_);
        size_ = 0;
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_);
    }

    void resize(size_type n, const_reference value = value_type()) {
        if (capacity_ == n) {
            while (begin_ != end_ + 1) {
                *end_++ = value;
            }
            size_ = capacity_;
            return;
        }
        pointer new_buff = alloc_.allocate(n + 1);
        for (size_t i = 0; i < n; i++) {
            if (i < size_) {
                std::construct_at(new_buff + i, begin_[i]);
            } else {
                std::construct_at(new_buff + i, value);
            }
        }
        this->clear();
        alloc_.deallocate(buff_, capacity_);
        buff_ = new_buff;
        capacity_ = n + 1;
        size_ = n;
        begin_ = iterator(new_buff, capacity_);
        end_ = iterator(new_buff, capacity_) + size_;
    }

    reference front() {
        if (empty()) {
            throw std::out_of_range("Tried to get an element from an empty buffer");
        }
        return *(this->begin());
    }

    const_reference front() const {
        if (empty()) {
            throw std::out_of_range("Tried to get an element from an empty buffer");
        }
        return *(this->begin());
    }

    reference back() {
        if (empty()) {
            throw std::out_of_range("Tried to get an element from an empty buffer");
        }
        auto tmp = this->end();
        --tmp;
        return *tmp;
    }

    const_reference back() const {
        if (empty()) {
            throw std::out_of_range("Tried to get an element from an empty buffer");
        }
        auto tmp = this->end();
        --tmp;
        return *tmp;
    }

    void push_back(const_reference value) {
        if (size_ == capacity_ - 1) {
            reserve((capacity_ - 1) * kCapacityCoefficient);
        }
        std::construct_at(&*end_, value);
        end_++;
        size_++;
    }

    void push_front(const_reference value) {
        if (size_ == capacity_ - 1) {
            reserve((capacity_ - 1) * kCapacityCoefficient);
        }
        begin_--;
        std::construct_at(&*begin_, value);
        size_++;
    }

    void pop_back() {
        if (empty()) {
            throw std::out_of_range("Tried to pop from an empty buffer");
        }
        end_--;
        std::destroy_at(&*end_);
        size_--;
    }

    void pop_front() {
        if (empty()) {
            throw std::out_of_range("Tried to pop from an empty buffer");
        }
        std::destroy_at(&*begin_);
        begin_++;
        size_--;
    }

    iterator erase(const_iterator q) {
        if (empty()) {
            throw std::out_of_range("Tried to erase from an empty buffer");
        }
        iterator it(buff_, capacity_, const_cast<T*>(q.cur_elem()));
        while (it + 1 != end_) {
            std::destroy_at(&*it);
            std::construct_at(&*it, *(it + 1));
            it++;
        }
        std::destroy_at(&*it);
        size_--;
        end_--;
        return iterator(buff_, capacity_, const_cast<T*>(q.cur_elem()));
    }

    iterator erase(const_iterator q1, const_iterator q2) {
        if (empty()) {
            throw std::out_of_range("Tried to erase from an empty buffer");
        }
        size_type len;
        if (q1 < q2) {
            len = q2 - q1;
        } else {
            len = capacity_ - (q1 - q2);
        }
        if (len > size_) {
            this->clear();
            return end_;
        }
        auto q = q1;
        while (q != q2) {
            std::destroy_at(&*q);
            q++;
        }
        iterator it(buff_, capacity_, const_cast<T*>(q2.cur_elem()));
        while (it != end_) {
            std::construct_at(&*(it - len), *it);
            std::destroy_at(&*it);
            it++;
        }
        size_ -= len;
        end_ -= len;
        iterator result(buff_, capacity_, const_cast<T*>(q2.cur_elem()));
        if ((end_ < begin_ && result >= end_ && result < begin_) ||
            (end_ > begin_ && (result >= end_ || result < begin_)) || empty()) {
            return end_;
        }
        return result;
    }

    template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>>
    void assign(InputIterator i, InputIterator j) {
        this->clear();
        alloc_.deallocate(buff_, capacity_);
        capacity_ = j - i + 1;
        size_ = capacity_ - 1;
        buff_ = alloc_.allocate(capacity_);
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
        auto this_it = begin_;
        auto other_it = i;
        while (other_it != j) {
            std::construct_at(&*this_it++, *other_it++);
        }
    }

    void assign(const std::initializer_list<value_type>& il) {
        assign(il.begin(), il.end());
    }

    void assign(size_type n, value_type t) {
        this->clear();
        alloc_.deallocate(buff_, capacity_);
        capacity_ = n + 1;
        size_ = capacity_ - 1;
        buff_ = alloc_.allocate(capacity_);
        for (size_t i = 0; i < n; i++) {
            std::construct_at(&buff_[i], t);
        }
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_) + size_;
    }

    reference at(size_type index) {
        if (empty()) {
            throw std::out_of_range("Tried to get an element from an empty buffer");
        }
        return begin_(index);
    }

    const_reference at(size_type index) const {
        if (empty()) {
            throw std::out_of_range("Tried to get an element from an empty buffer");
        }
        return begin_(index);
    }

    void reserve(size_type n) {
        if (n <= capacity_ - 1) {
            return;
        }
        auto old_buff = buff_;
        auto it = begin_;
        auto it_end = end_;
        auto old_cap = capacity_;
        auto old_size = size_;
        capacity_ = n + 1;
        buff_ = alloc_.allocate(capacity_);
        size_ = 0;
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_);
        while (it != it_end) {
            push_back(*it);
            it++;
        }
        std::destroy_n(old_buff, old_size);
        alloc_.deallocate(old_buff, old_cap);
    }

    iterator insert(const_iterator p, size_type n, value_type t) {
        if (n == 0) {
            return iterator(buff_, capacity_, const_cast<T*>(p.cur_elem()));
        }
        iterator result;
        auto old_buff = buff_;
        auto old_size = size_;
        auto it = begin_;
        auto it_end = end_;
        iterator it_p(buff_, capacity_, const_cast<T*>(p.cur_elem()));
        buff_ = alloc_.allocate(capacity_);
        size_ = 0;
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_);
        reserve(n + old_size);
        while (it != it_end) {
            push_back(*it);
            if (it + 1 == it_p) {
                result = end_;
                for (size_t i = 0; i < n; i++) {
                    push_back(t);
                }
            }
            it++;
        }
        std::destroy_n(old_buff, old_size);
        alloc_.deallocate(old_buff, capacity_);
        return result;
    }

    iterator insert(const_iterator p, value_type t) {
        return insert(p, 1, t);
    }

    template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>>
    iterator insert(const_iterator p, InputIterator i, InputIterator j) {
        if (i == j) {
            return iterator(buff_, capacity_, const_cast<T*>(p.cur_elem()));
        }
        iterator result;
        auto old_buff = buff_;
        auto old_size = size_;
        auto it = begin_;
        auto it_end = end_;
        iterator it_p(buff_, capacity_, const_cast<T*>(p.cur_elem()));
        buff_ = alloc_.allocate(capacity_);
        size_ = 0;
        begin_ = iterator(buff_, capacity_);
        end_ = iterator(buff_, capacity_);
        reserve(j - i + old_size);
        while (it != it_end) {
            push_back(*it);
            if (it + 1 == it_p) {
                result = end_;
                while (i != j) {
                    push_back(*i);
                    i++;
                }
            }
            it++;
        }
        std::destroy_n(old_buff, old_size);
        alloc_.deallocate(old_buff, capacity_);
        return result;
    }

    iterator insert(const_iterator p, const std::initializer_list<T>& il) {
        return insert(p, il.begin(), il.end());
    }
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const CCircularBufferExt<T>& a) {
    auto it = a.begin();
    for (size_t i = 0; i < a.size(); i++, it++) {
        os << *it << ' ';
    }
    return os;
}