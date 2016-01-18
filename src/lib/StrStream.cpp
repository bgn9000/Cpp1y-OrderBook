#include "StrStream.h"

#include "Decoder.h"

#include <cstring>

void StrStream::append(const char* str, size_t len)
{
    size_t addedLen = 0;
    auto appendBeyond = [&]()
    {
        const auto prevSizeOver = sizeOver_;
        sizeOver_ += (len - addedLen);
        if (sizeOver_ > CAPACITY_MAX) sizeOver_=CAPACITY_MAX;
        memcpy(strOver_+prevSizeOver, str+addedLen, sizeOver_-prevSizeOver);
        strOver_[sizeOver_]=0;
    };
    
    if (likely(strOver_ == nullptr))
    {
        size_t prevSize = size();
        resize(prevSize + len);
        addedLen = size() - prevSize;
        memcpy(begin() + prevSize, str, addedLen);

        if (unlikely(len > addedLen))
        {
            strOver_ = new char[CAPACITY_MAX+1];
            memcpy(strOver_, begin(), size());
            sizeOver_ += size();
            appendBeyond();
        }
    }
    else
    {
        appendBeyond();
    }
}

void StrStream::append(size_t pos, char c)
{
    size_t len = size();
    if (unlikely(pos < len)) return;
    auto appendBeyond = [&]()
    {
        const auto prevSizeOver = sizeOver_;
        sizeOver_ = pos;
        if (sizeOver_ > CAPACITY_MAX) sizeOver_=CAPACITY_MAX;
        memset(strOver_+prevSizeOver, c, sizeOver_-prevSizeOver);
        strOver_[sizeOver_]=0;
    };
    
    if (likely(strOver_ == nullptr))
    {
        if (likely(pos < capacity()))
        {
            resize(pos);
            memset(begin() + len, c, pos - len);
        }
        else
        {
            strOver_ = new char[CAPACITY_MAX+1];
            memcpy(strOver_, begin(), size());
            sizeOver_ += size();
            appendBeyond();
        }
    }
    else
    {
        appendBeyond();
    }
}

StrStream& StrStream::operator<<(const char* str)
{
    append(str, strlen(str));
    return *this;
}

StrStream& StrStream::operator<<(char c)
{
    size_t len = size();
    auto appendBeyond = [&]()
    {
        if (likely(sizeOver_ < CAPACITY_MAX))
        {
            strOver_[sizeOver_++]=c;
            strOver_[sizeOver_]=0;
        }
    };
    
    if (likely(strOver_ == nullptr))
    {
        if (likely(len < capacity()))
        {
            resize(len+1);
            *(begin()+len) = c;
        }
        else
        {
            strOver_ = new char[CAPACITY_MAX+1];
            memcpy(strOver_, begin(), size());
            sizeOver_ += size();
            appendBeyond();
        }
    }
    else
    {
        appendBeyond();
    }
    return *this;
}

StrStream& StrStream::operator<<(const StrStream& strstr)
{
    append(strstr.c_str(), strstr.length());
    return *this;
}

StrStream& StrStream::operator<<(std::ostream& ostr)
{
    if (unlikely(!ostr.rdbuf()))
    {
        return *this;
    }
    unsigned int x = 0;
    while (ostr.rdbuf()->sgetc()!=EOF)
    {
        operator<<(static_cast<char>(ostr.rdbuf()->sbumpc()));
        ++x;
    }
    while (x--)
    {
        ostr.rdbuf()->sungetc();
    }
    return *this;
}

std::ostream& operator<<(std::ostream& os, const StrStream& strstr)
{
    return os << strstr.c_str();
}

StrStream& StrStream::operator<<(float flt)
{
    char buf[64] = {};
    size_t bufsize = Decoder::convert_unsigned_float<float>(buf, flt, 3 /*std::numeric_limits<float>::digits10*/);
    append(buf, bufsize);
    return *this;
}

StrStream& StrStream::operator<<(double dbl)
{
    char buf[64] = {};
    size_t bufsize = Decoder::convert_unsigned_float<double>(buf, dbl, 6 /*std::numeric_limits<double>::digits10*/);
    append(buf, bufsize);
    return *this;
}

StrStream& StrStream::operator<<(unsigned int n)
{
    char buf[64] = {};
    size_t bufsize = Decoder::convert_unsigned_integer<unsigned int>(n, buf);
    append(buf, bufsize);
    return *this;
}

StrStream& StrStream::operator<<(unsigned long n)
{
    char buf[64] = {};
    size_t bufsize = Decoder::convert_unsigned_integer<unsigned long>(n, buf);
    append(buf, bufsize);
    return *this;
}

StrStream& StrStream::operator<<(unsigned long long n)
{
    char buf[64] = {};
    size_t bufsize = Decoder::convert_unsigned_integer<unsigned long long>(n, buf);
    append(buf, bufsize);
    return *this;
}

