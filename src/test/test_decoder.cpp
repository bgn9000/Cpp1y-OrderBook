#include <rapidcheck.h>

#include "Decoder.h"

#include <cmath>
#include <cstring>

#include <chrono>
using namespace std::chrono;

//#include <boost/lexical_cast.hpp>

int main()
{
    auto time_span1 = 0ULL, time_span2 = 0ULL;
    auto nbTests = 0U;
    rc::check("Reverse string", [&](std::string str) 
    {
        char* str2 = strdup(str.c_str());
        const auto len = strlen(str2);
        if (len > 0)
        {
            high_resolution_clock::time_point start = high_resolution_clock::now();
            std::reverse(std::begin(str), std::end(str)); 
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span1 += duration_cast<nanoseconds>(end - start).count();
            
            start = high_resolution_clock::now();
            Decoder::strreverse(str2, str2+len-1);
            end = high_resolution_clock::now();
            time_span2 += duration_cast<nanoseconds>(end - start).count();
            
            ++nbTests;
            
            RC_ASSERT(str2 == str);
        }
    });
    if (nbTests)
    {
        std::cout << "Reverse string perfs : std::reverse [" << time_span1/nbTests
            << "] Decoder::strreverse [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Parse unsigned integer", [&](unsigned int i) 
    {
        auto str = std::to_string(i);
        
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto ret = std::stoul(str);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
//        RC_ASSERT(ret == i);

        start = high_resolution_clock::now();
        ret = Decoder::retreive_unsigned_integer<unsigned int>(str.c_str(), str.length());
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
        
        RC_ASSERT(ret == i);
    });
    if (nbTests)
    {
        std::cout << "Parse unsigned integer perfs : std::stoul [" << time_span1/nbTests
            << "] Decoder::retreive_unsigned_integer [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Convert integer to string", [&](unsigned int i) 
    {
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto str = std::to_string(i);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        char buf[64] = {};
        auto len = Decoder::convert_unsigned_integer<unsigned int>(i, buf);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
        
        RC_ASSERT(buf == str);
    });
    if (nbTests)
    {
        std::cout << "Convert integer to string perfs : std::to_string [" << time_span1/nbTests
            << "] Decoder::convert_unsigned_integer [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Parse double", [&](double d)
    {
        char buf[64] = {};
        size_t size = Decoder::convert_float<double>(buf, d, std::numeric_limits<double>::digits10);

        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto ret = std::stod(std::string(buf, size));
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
//        RC_ASSERT(ret - d < std::pow(10, -std::numeric_limits<double>::digits10));
        
        start = high_resolution_clock::now();
        ret = Decoder::retreive_float<double>(buf, size);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();

        double diff = ret - d;
        
        RC_LOG() << std::fixed << std::setprecision(std::numeric_limits<double>::digits10) 
            << "buf [" << buf << "] and d [" << d << "] ret [" << ret << "] diff [" << diff << "]" << std::endl;
        
        ++nbTests;
        
        RC_ASSERT(diff < std::pow(10, -std::numeric_limits<double>::digits10));
    });
    if (nbTests)
    {
        std::cout << "Parse double perfs : std::stod [" << time_span1/nbTests
            << "] Decoder::retreive_float [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }

    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Convert double to string", [&](double d) 
    {
        high_resolution_clock::time_point start = high_resolution_clock::now();
//        auto str = std::to_string(d); // only 6 digits precision
        char buf[64] = {};
        sprintf(buf, "%.*e", std::numeric_limits<double>::digits10, d);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        char buf2[64] = {};
        size_t size = Decoder::convert_float<double>(buf2, d, std::numeric_limits<double>::digits10);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();

        ++nbTests;
                
        RC_ASSERT(std::string(buf2, size) == buf2);
    });
    if (nbTests)
    {
        std::cout << "Convert double to string perfs : sprintf [" << time_span1/nbTests
            << "] Decoder::convert_float [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    return 0;
}

