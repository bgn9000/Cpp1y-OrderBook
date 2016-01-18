#pragma once

#include "Common.h"

#include <atomic>
#include <array>
#include <mutex>

using namespace common;

// !! Only One publisher / One Listener !!
// Could consider yielding instead of spin loop

template <typename T, size_t _BlockCapacity = 1024>
class CircularBlock
{
public:
    static constexpr size_t CAPACITY = _BlockCapacity;
    static_assert(((CAPACITY > 0) && ((CAPACITY & (~CAPACITY + 1)) == CAPACITY)), "Block capacity must be a positive power of 2");
    
    static auto capacity()
    {
        return CAPACITY;
    }
    
    CircularBlock() = default;
    ~CircularBlock() = default;
    CircularBlock(const CircularBlock&) = delete;
    CircularBlock& operator=(const CircularBlock&) = delete;
    
    void fill(T&& data)
    {
        while (size_.load(std::memory_order::memory_order_acquire) >=  CAPACITY-1) // spin loop
            if (unlikely(dontSpin_)) return;
        if (++last_ == CAPACITY) last_ = 0;
        array_[last_] = std::forward<T>(data);
        size_.fetch_add(1, std::memory_order::memory_order_release);
    }
    
    auto empty()
    {
        while (size_.load(std::memory_order::memory_order_acquire) == 0) // spin loop
            if (unlikely(dontSpin_)) return T();
        if (++first_ == CAPACITY) first_ = 0;
        size_.fetch_sub(1, std::memory_order::memory_order_release);
        return array_[first_];
    }
    
    void dontSpin() { dontSpin_ = true; }

protected:
    bool dontSpin_ = false;
    size_t first_ = CAPACITY-1;
    size_t last_ = CAPACITY-1;
    std::atomic<size_t> size_{0UL};
    
//    std::array<T, CAPACITY> array_;
    std::vector<T> array_{CAPACITY};
};
