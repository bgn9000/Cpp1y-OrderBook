#pragma once

#include "utils/Common.h"
using namespace common;

#include <cstring>

class SimpleBuffer
{
protected :
    size_t capacity_ = 0;
    char* str_ = nullptr;
    bool toDelete = false;

    size_t begin_ = 0;
    size_t end_ = 0;

public :
    static constexpr size_t SIMPLE_BUFFER_SIZE = (1024 * 1000);
    
    SimpleBuffer(size_t capacity = SIMPLE_BUFFER_SIZE)
        : capacity_(capacity), str_(new char[capacity]), toDelete(true)
    {
    }
    SimpleBuffer(char* str, size_t len)
        : capacity_(len), str_(str), toDelete(false)
    {
    }
    SimpleBuffer(const SimpleBuffer&) = delete;
    SimpleBuffer& operator=(const SimpleBuffer&) = delete;
    ~SimpleBuffer()
    {
        if (unlikely(toDelete)) delete[] str_;
    }
    
    void push(const char* str, size_t len)
    {
        memcpy(str_+end_,str,len);
        end_ += len;
    }

    void reset()
    {
        begin_ = end_ = 0;
    }

    void pushOnLeft()
    {
        memmove(str_, str_+begin_, end_-begin_);
        end_ -= begin_;
        begin_=0;
    }

    size_t available() const
    {
        return (end_-begin_);
    }

    size_t freeSpace() const
    {
        return (capacity_-end_);
    }

    // We assume we never need this :
    //      Initial capacity setted to SIMPLE_BUFFER_SIZE (by default)
    //      If not enough, double it
    /*void moreSpace()
    {
        char* prevStr = str_;
        size_t prevCapacity_ = capacity_;
        capacity_ *= 2;
        str_ = new char[capacity_];
        memcpy(str_+end_, prevStr+end_, prevCapacity_-end_);
        delete [] prevStr;
    }*/

    size_t totalCapacity() const
    {
        return capacity_;
    }

    void seek(size_t step)
    {
        begin_+=step;
        if (begin_ > end_)
            begin_=end_;
    }
    
    FORCE_INLINE int getPosition(const char c)
    {
        auto pos = begin_;
        for (; pos < end_; ++pos) 
        {
            if (c == str_[pos])
            {
                return static_cast<int>(pos-begin_);
            }
        }
        return -1;
    }

    void seekEnd(size_t step)
    {
        end_ += step;
    }

    char* dataEnd() const
    {
        return (str_+end_);
    }

    char* data() const
    {
        return (str_+begin_);
    }

    char& operator[](size_t i)
    {
        return str_[begin_+i];
    }
};

