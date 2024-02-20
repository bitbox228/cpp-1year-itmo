#pragma once

#include <iostream>
#include <stdexcept>
#include <vector>
#include <array>

template<typename T>
class PoolAllocator {
public:
    using size_type = size_t;
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using difference_type = std::make_signed_t<size_t>;
    using pair = std::pair<size_t, size_t>;

    PoolAllocator(const std::initializer_list<pair>& il) : pools_count_(il.size()) {
        std::vector<pair> vec(il);
        std::sort(vec.begin(), vec.end());
        pools_ = new Pool[pools_count_];
        for (auto it = vec.begin(); it != vec.end(); it++) {
            pools_[it - vec.begin()] = Pool(it->first, it->second);
        }
    }

    template<typename S>
    struct rebind {
        using other = PoolAllocator<S>;
    };

    template<typename S>
    explicit PoolAllocator(const PoolAllocator<S>& other) : pools_(reinterpret_cast<Pool*>(other.pools())),
                                                            pools_count_(other.pools_count()) {}

    pointer allocate(size_type n) {
        difference_type index = PoolSearch(n);
        if (index != -1) {
            for (size_type i = index; i < pools_count_; ++i) {
                if (pools_[i].IsAllocatable(sizeof(value_type), n)) {
                    auto result = reinterpret_cast<pointer>(pools_[i].AllocateChunks(sizeof(value_type), n));
                    return result;
                }
            }
        }
        for (size_type i = 0; i < pools_count_; i++) {
            if (pools_[pools_count_ - i - 1].IsAllocatable(sizeof(value_type), n)) {
                auto result = reinterpret_cast<pointer>(pools_[pools_count_ - i - 1].AllocateChunks(sizeof(value_type),
                                                                                                    n));
                return result;
            }
        }
        std::cout << n * sizeof(T) << ' ';
        throw std::bad_alloc();
    }

    void deallocate(pointer p, size_type n) {
        Pool* pool = nullptr;
        for (size_t i = 0; i < pools_count_; i++) {
            if (pools_[i].IsPointerInPool(p, n)) {
                pool = &pools_[i];
                break;
            }
        }
        if (pool == nullptr) {
            return;
        }
        pool->DeallocateChunks(p, n);
    }

    bool operator!=(const PoolAllocator<T>& other) const {
        if (other.pools_count_ != pools_count_) {
            return false;
        }
        for (size_type i = 0; i < pools_count_; i++) {
            if (pools_[i] != other.pools_[i]) {
                return false;
            }
        }

        return true;
    };

    bool operator==(const PoolAllocator<T>& other) const {
        return !(*this == other);
    };

private:

    difference_type PoolSearch(size_type n) {
        difference_type l = -1;
        difference_type r = pools_count_;
        while (r > l + 1) {
            difference_type m = (l + r) / 2;
            if (pools_[m].chunks_size() >= sizeof(T) * n) {
                r = m;
            } else {
                l = m + 1;
            }
        }
        if ((r < pools_count_) && (pools_[r].chunks_size() >= sizeof(T) * n)) {
            return r;
        } else {
            return -1;
        }
    }

    class Pool {
        size_t chunks_size_;
        size_t chunks_count_;
        size_t remainder_;
        uint8_t* chunks_;
        bool* chunks_occupancy_;

    public:
        Pool() = default;

        Pool(size_t chunks_size, size_t chunks_count) :
                chunks_size_(chunks_size), chunks_count_(chunks_count), remainder_(chunks_count_) {
            chunks_ = static_cast<uint8_t*>(malloc(chunks_size_ * chunks_count_));
            chunks_occupancy_ = static_cast<bool*>(malloc(chunks_count_));
            for (size_t i = 0; i < chunks_count_; i++) {
                chunks_occupancy_[i] = true;
            }
        }

        size_t chunks_size() {
            return chunks_size_;
        }

        size_t chunks_count() {
            return chunks_count_;
        }

        bool operator!=(const Pool& other) const {
            return (chunks_size_ == other.chunks_size_ && chunks_count_ == other.chunks_count_ &&
                    remainder_ == other.remainder_);
        }

        bool operator==(const Pool& other) const {
            return !(*this == other);
        }

        template<typename S>
        bool IsPointerInPool(S* ptr, size_t n) {
            size_t chunks_needed = (sizeof(S) * n) / chunks_size_ + ((sizeof(S) * n) % chunks_size_ != 0);
            auto cur_ptr = reinterpret_cast<uint8_t*>(ptr);
            auto end_ptr = cur_ptr + chunks_needed * chunks_size_;
            return (cur_ptr >= chunks_ && cur_ptr < chunks_ + (chunks_size_ * chunks_count_)) &&
                   (end_ptr <= chunks_ + (chunks_size_ * chunks_count_));
        }

        bool IsAllocatable(size_t type_size, size_t n) {
            size_t chunks_needed;
            if (type_size * n >= chunks_size_ && remainder_ == 0) {
                return false;
            } else {
                chunks_needed = (type_size * n) / chunks_size_ + ((type_size * n) % chunks_size_ != 0);
            }
            if (chunks_needed > remainder_) {
                return false;
            }
            if (chunks_needed == 1) {
                return true;
            }
            size_t current_row = 0;
            for (size_t i = 0; i < chunks_count_; i++) {
                if (chunks_occupancy_[i]) {
                    current_row++;
                    if (current_row == chunks_needed) {
                        return true;
                    }
                } else {
                    current_row = 0;
                }
            }

            return false;
        }

        uint8_t* AllocateChunks(size_t type_size, size_t n) {
            size_t chunks_needed;
            if (type_size * n >= chunks_size_ && remainder_ == 0) {
                return nullptr;
            } else {
                chunks_needed = (type_size * n) / chunks_size_ + ((type_size * n) % chunks_size_ != 0);
            }
            if (chunks_needed > remainder_) {
                return nullptr;
            }
            size_t current_row = 0;
            uint8_t* ptr = chunks_;
            for (size_t i = 0; i < chunks_count_; i++) {
                if (current_row == 0) {
                    ptr = &chunks_[i * chunks_size_];
                }
                if (chunks_occupancy_[i]) {
                    current_row++;
                    if (current_row == chunks_needed) {
                        remainder_ -= chunks_needed;
                        for (size_t j = i + 1 - chunks_needed; j <= i; j++) {
                            chunks_occupancy_[j] = false;
                        }
                        return ptr;
                    }
                } else {
                    current_row = 0;
                }
            }

            return nullptr;
        }

        template<typename S>
        void DeallocateChunks(S* ptr, size_t n) {
            size_t chunks_needed = (sizeof(S) * n) / chunks_size_ + ((sizeof(S) * n) % chunks_size_ != 0);
            auto cur_ptr = reinterpret_cast<uint8_t*>(ptr);
            auto cur_ptr_occupancy = &chunks_occupancy_[(cur_ptr - chunks_) / chunks_size_];
            for (size_t i = 0; i < chunks_needed; i++) {
                cur_ptr_occupancy[i] = true;
            }
            remainder_ += chunks_needed;
        }
    };

private:
    Pool* pools_;
    size_type pools_count_;
public:
    Pool* pools() const {
        return pools_;
    }

    size_type pools_count() const {
        return pools_count_;
    }
};