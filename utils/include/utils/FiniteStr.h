#pragma once

#include "utils/Common.h"

#include <string>
#include <limits>

template<size_t _StringCapacity = 1024>
class FiniteStr
{
    // If this fails to compile, that means that buffer size is too big
    // (probably a conversion from a negative number)...
    typedef char AssertNotTooLargeCapacity[ _StringCapacity > (std::numeric_limits<size_t>::max() / 4) ? -1 : 1 ];

public:
    static constexpr size_t CAPACITY = _StringCapacity;

    FiniteStr() : size_(0)
    {
        str_[0] = 0;
    }
    FORCE_INLINE FiniteStr(char filler, size_t size);
    ~FiniteStr() = default;
    FiniteStr(const FiniteStr&) = delete;
    FiniteStr& operator=(const FiniteStr&) = delete;
    
    FORCE_INLINE size_t size() const
    {
        return size_;
    }
    FORCE_INLINE static size_t capacity()
    {
        return CAPACITY;
    }
    FORCE_INLINE void resize(size_t newSize);
    FORCE_INLINE void resize(size_t newSize, char filler);

    FORCE_INLINE void clear()
    {
        resize(0);
    }

    FORCE_INLINE const char* c_str() const
    {
        return str_;
    }

    FORCE_INLINE std::string toString() const;

    FORCE_INLINE char* begin()
    {
        return &str_[0];
    }
    FORCE_INLINE char* end()
    {
        return &str_[ size_ ];
    }
    
    FORCE_INLINE const char* begin() const
    {
        return &str_[0];
    }
    FORCE_INLINE const char* end() const
    {
        return &str_[ size_ ];
    }

    FORCE_INLINE void assign(const char* str, size_t len);

private:
    size_t size_;

    char   str_[ CAPACITY + 1 ]; // + 1 is for the trailing '\0'
};

template<size_t _capacity> FORCE_INLINE
FiniteStr<_capacity>::FiniteStr(char filler, size_t size) :
    size_(size > CAPACITY ? CAPACITY : size)
{
    memset(str_, filler, size_);
    str_[size_] = 0;
}

template<size_t _capacity> FORCE_INLINE
void FiniteStr<_capacity>::resize(size_t newSize)
{
    size_ = (newSize > CAPACITY ? CAPACITY : newSize);
    str_[ size_ ] = 0;
}

template<size_t _capacity> FORCE_INLINE
void FiniteStr<_capacity>::resize(size_t newSize, char filler)
{
    size_t prevSize = size_;
    size_ = (newSize > CAPACITY ? CAPACITY : newSize);
    while (prevSize < size_)
    {
        str_[ prevSize++ ] = filler;
    }
    str_[ size_ ] = 0;
}

template<size_t _capacity> FORCE_INLINE
std::string FiniteStr<_capacity>::toString() const
{
    return std::string(str_, size_);
}

template<size_t _capacity> FORCE_INLINE
void FiniteStr<_capacity>::assign(const char* str, size_t len)
{
    resize(len);
    memcpy(str_, str, size_);
}

