#include <rapidcheck.h>

#include "utils/Decoder.h"

#include <cmath>
#include <cstring>

#include <chrono>
using namespace common;

//#include <boost/lexical_cast.hpp>

int main()
{
    using std::chrono::high_resolution_clock;
    high_resolution_clock::time_point start, end;
    using std::chrono::nanoseconds;
    using std::chrono::duration_cast;
    auto time_span1 = 0ULL, time_span2 = 0ULL;
    auto nbTests = 0U;
    
    rc::check("Reverse string", [&](std::string str) 
    {
        char* str2 = strdup(str.c_str());
        const auto len = strlen(str2);
        if (len > 0)
        {
            start = high_resolution_clock::now();
            std::reverse(std::begin(str), std::end(str)); 
            end = high_resolution_clock::now();
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
    rc::check("Parse Quantity", [&](Quantity i) 
    {
        auto str = std::to_string(i);
        
        start = high_resolution_clock::now();
        auto ret = std::stoul(str);
        end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
//        RC_ASSERT(ret == i);

        start = high_resolution_clock::now();
        ret = Decoder::retreive_unsigned_integer<Quantity>(str.c_str(), str.length());
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
        
        RC_ASSERT(ret == i);
    });
    if (nbTests)
    {
        std::cout << "Parse Quantity perfs : std::stoul [" << time_span1/nbTests
            << "] Decoder::retreive_unsigned_integer [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Convert Quantity to string", [&](Quantity i) 
    {
        start = high_resolution_clock::now();
        auto str = std::to_string(i);
        end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        char buf[64] = {};
        auto len = Decoder::convert_unsigned_integer<Quantity>(i, buf);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
        
        RC_ASSERT(buf == str);
    });
    if (nbTests)
    {
        std::cout << "Convert Quantity to string perfs : std::to_string [" << time_span1/nbTests
            << "] Decoder::convert_unsigned_integer [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Parse Price", [&]()
    {
        auto d = *rc::gen::positive<Price>();
        char buf[64] = {};
        size_t size = Decoder::convert_unsigned_float<Price>(buf, d, std::numeric_limits<Price>::digits10);

        std::string strBuf(buf, size);
        start = high_resolution_clock::now();
        auto ret = std::stod(strBuf);
        end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
//        RC_ASSERT(ret - d < std::pow(10, -std::numeric_limits<Price>::digits10));
        
        start = high_resolution_clock::now();
        ret = Decoder::retreive_unsigned_float<Price>(buf, size);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();

        Price diff = ret - d;
        
        RC_LOG() << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10) 
            << "buf [" << buf << "] and d [" << d << "] ret [" << ret << "] diff [" << diff << "]" << std::endl;
        
        ++nbTests;
        
        RC_ASSERT(diff < std::pow(10, -std::numeric_limits<Price>::digits10));
    });
    if (nbTests)
    {
        std::cout << "Parse Price perfs : std::stod [" << time_span1/nbTests
            << "] Decoder::retreive_float [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Parse leading '0' Price", [&]()
    {
        auto d = *rc::gen::positive<Price>();
        char buf[64] = {};
        size_t size = Decoder::convert_unsigned_float<Price>(buf, d, std::numeric_limits<Price>::digits10);
        auto nbLeadingZeros = *rc::gen::inRange<unsigned int>(1, sizeof(buf)-strlen(buf)-1);
        std::string strBuf(nbLeadingZeros, '0');
        
        // only zeros
        auto ret = Decoder::retreive_unsigned_float<Price>(strBuf.c_str(), strBuf.length());
        
        RC_LOG() << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10) 
            << "buf [" << strBuf << "] ret [" << ret << "]" << std::endl;
            
        RC_ASSERT(0.0 == ret);
        
        strBuf += buf;
        
        start = high_resolution_clock::now();
        ret = std::stod(strBuf);
        end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
//        RC_ASSERT(ret - d < std::pow(10, -std::numeric_limits<Price>::digits10));
        
        start = high_resolution_clock::now();
        ret = Decoder::retreive_unsigned_float<Price>(strBuf.c_str(), strBuf.length());
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();

        Price diff = ret - d;
        
        RC_LOG() << "buf [" << strBuf << "] and d [" << d << "] ret [" << ret << "] diff [" << diff << "]" << std::endl;
        
        ++nbTests;
        
        RC_ASSERT(diff < std::pow(10, -std::numeric_limits<Price>::digits10));
    });
        if (nbTests)
    {
        std::cout << "Parse leading '0' Price perfs : std::stod [" << time_span1/nbTests
            << "] Decoder::retreive_float [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Convert Price to string", [&]() 
    {
        auto d = *rc::gen::positive<Price>();
        
        start = high_resolution_clock::now();
//        auto str = std::to_string(d); // only 6 digits precision
        char buf[64] = {};
        sprintf(buf, "%.*e", std::numeric_limits<Price>::digits10, d);
        end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        char buf2[64] = {};
        size_t size = Decoder::convert_unsigned_float<Price>(buf2, d, std::numeric_limits<Price>::digits10);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();

        ++nbTests;
                
        RC_ASSERT(std::string(buf2, size) == buf2);
    });
    if (nbTests)
    {
        std::cout << "Convert Price to string perfs : sprintf [" << time_span1/nbTests
            << "] Decoder::convert_float [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    return 0;
}

