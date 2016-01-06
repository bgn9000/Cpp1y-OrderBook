#include "StrStream.h"

#include "Decoder.h"

#include <cstring>

void StrStream::append(const char* str, size_t len)
{
    size_t addedLen = 0;
    if (likely(strOver_ == 0))
    {
        size_t prevSize = size();
        resize(prevSize + len);
        addedLen = size() - prevSize;
        memcpy(begin() + prevSize, str, addedLen);

        if (len > addedLen)
        {
            strOver_ = static_cast<char*>(malloc(CAPACITY_MAX*sizeof(char)+1));
            memcpy(strOver_, begin(), size());
            sizeOver_ += size();
        }
    }
    if (unlikely(strOver_ != 0))
    {
        size_t prevSizeOver = sizeOver_;
        sizeOver_ += (len - addedLen);
        if (sizeOver_ > CAPACITY_MAX) sizeOver_=CAPACITY_MAX;

        size_t addedLenOver = sizeOver_ - prevSizeOver;

        memcpy(strOver_ + prevSizeOver, str + addedLen, addedLenOver);
        strOver_[sizeOver_]=0;
    }
}

void StrStream::append(size_t pos, char c)
{
    size_t sz = size();
    if (unlikely(pos < sz || pos >= CAPACITY_MAX)) return;
    resize(pos);
    memset(begin() + sz, c, pos-sz);
}

StrStream& StrStream::operator<<(const char* str)
{
    append(str, strlen(str));
    return *this;
}

StrStream& StrStream::operator<<(char c)
{
    size_t sz = size();
    if (likely(strOver_ == 0))
    {
        if (sz < capacity())
        {
            resize(sz + 1);
            *(begin() + sz) = c;
        }
        else
        {
            strOver_ = static_cast<char*>(malloc(CAPACITY_MAX*sizeof(char)+1));
            memcpy(strOver_, begin(), size());
            sizeOver_ += size();
        }
    }
    if (unlikely(strOver_ != 0))
    {
        if (sizeOver_ < CAPACITY_MAX)
        {
            strOver_[sizeOver_++]=c;
            strOver_[sizeOver_]=0;
        }
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

