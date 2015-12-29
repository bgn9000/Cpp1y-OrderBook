#pragma once

#include "FiniteStr.h"
#include "Common.h"

using namespace common;

#include <iostream>

class StrStream : public FiniteStr<>
{
public:
    static constexpr size_t CAPACITY_MAX = 32768;
    
    StrStream() = default;
    StrStream(const StrStream&) = delete;
    StrStream& operator=(const StrStream&) = delete;
    ~StrStream() { clear(); }
    
    void clear()
    {
        FiniteStr<>::clear();
        if (unlikely(strOver_ != 0))
        {
            free(strOver_);
            strOver_ = 0;
        }
    }
    
    const char* c_str() const
    {
        if (likely(strOver_ == 0))
        {
            return FiniteStr<>::c_str();
        }
        return strOver_;
    }
    size_t length() const
    {
        if (likely(strOver_ == 0))
        {
            return size();
        }
        return sizeOver_;
    }
    
    void append(const char* str, size_t len);
    
    //AVOID THIS (use append (faster) or operator<< on const char*)
    StrStream& operator<<(const std::string& str) = delete;
    
    StrStream& operator<<(const char* str);
    StrStream& operator<<(char c);
    
    StrStream& operator<<(const StrStream& strstr);
    
    StrStream& operator<<(std::ostream& ostr);
    friend std::ostream& operator<<(std::ostream&, const StrStream&);
    
    StrStream& operator<<(float flt);
    StrStream& operator<<(double dbl);
    StrStream& operator<<(unsigned int n);
    StrStream& operator<<(unsigned long long n);
    
private:
    char*  strOver_ = 0;
    size_t sizeOver_ = 0;
};
