#pragma once

#include "Common.h"

namespace Decoder
{
    static const char* digit_lookup_table =
    {
        "00102030405060708090" \
        "01112131415161718191" \
        "02122232425262728292" \
        "03132333435363738393" \
        "04142434445464748494" \
        "05152535455565758595" \
        "06162636465666768696" \
        "07172737475767778797" \
        "08182838485868788898" \
        "09192939495969798999"
    };

    inline void strreverse(char* begin, char* end)
    {
        while (end > begin)
        {
            const char reverse = *end;
            *end = *begin;
            --end;
            *begin = reverse;
            ++begin;
        }
    }
    
    template <typename T>
    inline size_t convert_unsigned_integer(T val, char* str)
    {
        char* const begin = str;
        const char* entry;

        // Conversion. Number is reversed.
        while (val > 99)
        {
            const T tmp(val / 100);
            entry = &digit_lookup_table[(val - 100 * tmp) << 1];
            *str = *entry;
            ++str;
            *str = *(entry + 1);
            ++str;
            val = tmp;
        }

        // Convert remaining one or two digits.
        entry = &digit_lookup_table[val << 1];
        if (val > 9)
        {
            *str = *entry;
            ++str;
            ++entry;
        }
        *str = *entry;
        ++str;
        *str=0;

        strreverse(begin, str-1);

        return str-begin;
    }

    template <typename T>
    inline T retreive_unsigned_integer(const char* str, size_t size)
    {
        T val = 0;
        for (; size; --size, ++str)
        {
            val *= 10;
            val += *str - '0';
        }
        return val;
    }
    
    template <typename T>
    inline size_t convert_float(char* str, T val, int precision)
    {
        int pos = 0;
        if (val < 0)
        {
            val = -val;
            str[pos] = '-';
            ++pos;
        }

        long long integerPart = (long long)val;
        val -= integerPart;

        long long decimalMax = 1;
        for (int tmp = precision; tmp > 0; --tmp)
        {
            val *= 10;
            decimalMax *= 10;
        }
        val += 0.5;

        long long decimal = (long long)val;
        // overflow?
        if (decimal >= decimalMax)
        {
            // increment the integer part and decrement the decimal ones
            ++integerPart;
            decimal -= decimalMax;
        }

        // Writing decimal part in reverse order
        int pos2 = pos;
        if (decimal != 0)
        {
            const char* entry;
            
            // Conversion. Number is reversed.
            while (decimal > 99)
            {
                const long long tmp(decimal / 100);
                entry = &digit_lookup_table[(decimal - 100 * tmp) << 1];            
                str[pos2] = *entry;
                ++pos2;
                str[pos2] = *(entry + 1);
                ++pos2;
                decimal = tmp;
                precision -= 2;
            }

            // Convert remaining one or two digits.
            entry = &digit_lookup_table[decimal << 1];
            if (decimal > 9)
            {
                str[pos2] = *entry;
                ++pos2;
                ++entry;
                --precision;
            }
            str[pos2] = *entry;
            ++pos2;
            --precision;
        }
        // Add some missing '0'
        for (int tmp = precision; tmp != 0; --tmp)
        {
            str[pos2] = '0';
            ++pos2;
        }

        // Writing the dot
        str[pos2] = '.';
        ++pos2;

        // Writing integer part in reverse order
        if (integerPart != 0)
        {
            const char* entry;
            
            // Conversion. Number is reversed.
            while (integerPart > 99)
            {
                const long long tmp(integerPart / 100);
                entry = &digit_lookup_table[(integerPart - 100 * tmp) << 1];
                str[pos2] = *entry;
                ++pos2;
                str[pos2] = *(entry + 1);
                ++pos2;
                integerPart = tmp;
            }

            // Convert remaining one or two digits.
            entry = &digit_lookup_table[integerPart << 1];
            if (integerPart > 9)
            {
                str[pos2] = *entry;
                ++pos2;
                ++entry;
            }
            str[pos2] = *entry;
            ++pos2;
        }
        else
        {
            str[pos2] = '0';
            ++pos2;
        }
        
        strreverse(str+pos, str+pos2-1);

        return pos2;
    }
    
    template <typename T>
    inline T retreive_float(const char* str, size_t size)
    {
        if (size == 0) return 0;
        
        // sign
        int coef = 1;
        if (*str == '-')
        {
            coef = -1;
            --size;
            if (size == 0) return 0;
            ++str;
        }
        else if (*str == '+')
        {
            coef = 1;
            --size;
            if (size == 0) return 0;
            ++str;
        }
        
        // Integer part
        long long integerPart = 0;
        for (; size && *str && *str != '.' ; --size, ++str)
        {
            integerPart *= 10;
            integerPart += *str - '0';
        }
        T num = static_cast<double>(integerPart);

        // Decimal
        if (size && *str == '.')
        {
            --size;
            ++str;

            T reminder = 0;
            int decimalSize = 0;
            for (; size && *str ; --size, ++str)
            {
                reminder *= 10;
                reminder += *str - '0';
                ++decimalSize;
            }

            for (; decimalSize; --decimalSize)
                reminder /= 10;

            num += reminder;
        }
        return num * coef;
    }
}
