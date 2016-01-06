#include <rapidcheck.h>

#include "StrStream.h"

#include <cmath>
#include <sstream>

#include <chrono>
using namespace std::chrono;

int main()
{
    auto time_span1 = 0ULL, time_span2 = 0ULL;
    auto nbTests = 0U;
    rc::check("Append a string to a string (< 512 characters)", [&](std::string strOrigin, std::string strToAppend) 
    {
        auto len = strOrigin.length() + strToAppend.length();
        while (len > FiniteStr<>::capacity())
        {
            strOrigin = *rc::gen::arbitrary<std::string>();
            strToAppend = *rc::gen::arbitrary<std::string>();
            len = strOrigin.length() + strToAppend.length();
        }
        
        high_resolution_clock::time_point start = high_resolution_clock::now();
        StrStream strstream;
        strstream.append(strOrigin.c_str(), strOrigin.length());
        strstream.append(strToAppend.c_str(), strToAppend.length());
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        std::string str(strOrigin);
        str += strToAppend;
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
        
        RC_ASSERT(strstream.c_str() == str);
    });
    if (nbTests)
    {
        std::cout << "Append a string to a string (< 512 characters) perfs  [" << time_span1/nbTests 
            << "] std::string [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Consider sputn", [&](std::string str) 
    {        
        std::ofstream null("/dev/null");
        null.sync_with_stdio(false);
        high_resolution_clock::time_point start = high_resolution_clock::now();
        null.rdbuf()->sputn(str.c_str(), str.length());
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        null.flush();
        
        ++nbTests;
    });
    rc::check("instead of operator<<", [&](std::string str) 
    {        
        std::ofstream null("/dev/null");
        null.sync_with_stdio(false);
        high_resolution_clock::time_point start = high_resolution_clock::now();
        null << str;
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        null.flush();
        
        ++nbTests;
    });
    nbTests /= 2;
    if (nbTests)
    {
        std::cout << "Consider sputn perfs  [" << time_span1/nbTests 
            << "] instead of [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Consider no flush at each output", [&](std::string str) 
    {        
        std::ofstream null("/dev/null");
        null.sync_with_stdio(false);
        high_resolution_clock::time_point start = high_resolution_clock::now();
        null.rdbuf()->sputn(str.c_str(), str.length());
        null.flush();
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Consider no flush at each output perfs  [" << time_span1/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Append a string to a string (> 512 characters)", [&]() 
    {
        auto size = *rc::gen::inRange<int>(0, 512);
        std::string strOrigin = *rc::gen::container<std::string>(size, rc::gen::character<typename std::string::value_type>());
        size = *rc::gen::inRange<int>(513, StrStream::CAPACITY_MAX-size-1);
        auto strToAppend = *rc::gen::container<std::string>(size, rc::gen::character<typename std::string::value_type>());
    
        high_resolution_clock::time_point start = high_resolution_clock::now();
        StrStream strstream;
        strstream.append(strOrigin.c_str(), strOrigin.length());
        strstream.append(strToAppend.c_str(), strToAppend.length());
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        std::string str(strOrigin);
        str += strToAppend;
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        RC_LOG() /*std::cout*/
            << "Append a string to a string (> 512 characters) strOrigin length [" << strOrigin.length() 
            << "] strToAppend length [" << strToAppend.length() << "] strstream length [" << strstream.length()
            << "] string length [" << str.length() << "]" << std::endl;
        
        ++nbTests;
        
        RC_ASSERT(strstream.c_str() == str);
    });
    if (nbTests)
    {
        std::cout << "Append a string to a string (> 512 characters) perfs  [" << time_span1/nbTests 
            << "] std::string [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Append an unsigned int to a string (< 512 characters)", [&](std::string strOrigin, const unsigned int intToAppend)
    {
        auto len = strOrigin.length();
        while (len > FiniteStr<>::capacity() - 64)
        {
            strOrigin = *rc::gen::arbitrary<std::string>();
            len = strOrigin.length();
        }
        
        high_resolution_clock::time_point start = high_resolution_clock::now();
        StrStream strstream;
        strstream.append(strOrigin.c_str(), strOrigin.length());
        strstream << intToAppend;
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        std::stringstream sstr;
        sstr << strOrigin;
        sstr << intToAppend;
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
        
        RC_ASSERT(strstream.c_str() == sstr.str());
    });
    if (nbTests)
    {
        std::cout << "Append an unsigned int to a string (< 512 characters) perfs  [" << time_span1/nbTests 
            << "] std::stringstream [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Append a double to a string (< 512 characters)", [&](std::string strOrigin) 
    {
        const auto doubleToAppend = *rc::gen::positive<double>();
                
        auto len = strOrigin.length();
        while (len > FiniteStr<>::capacity() - 64)
        {
            strOrigin = *rc::gen::arbitrary<std::string>();
            len = strOrigin.length();
        }
        
        high_resolution_clock::time_point start = high_resolution_clock::now();
        StrStream strstream;
        strstream.append(strOrigin.c_str(), strOrigin.length());
        strstream << doubleToAppend;
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        std::stringstream sstr;
        sstr << std::fixed /*<< std::setprecision(std::numeric_limits<double>::digits10)*/ << strOrigin;
        sstr << doubleToAppend;
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "Append a double to a string (< 512 characters) doubleToAppend [" << doubleToAppend 
            << "] strOrigin [" << strOrigin << "] strstream [" << strstream << "] stringstream [" << sstr.str()
            << "]" << std::endl;
        
        ++nbTests;
        
        const std::string cmp1(strstream.c_str(), strstream.length()-1);
        const std::string cmp2(sstr.str().c_str(), sstr.str().length()-1);
        RC_ASSERT(cmp1 == cmp2);
    });
    if (nbTests)
    {
        std::cout << "Append a double to a string (< 512 characters) perfs  [" << time_span1/nbTests 
            << "] std::stringstream [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
}

