#include <rapidcheck.h>

#include "utils/StrStream.h"

#include <cmath>
#include <sstream>
#include <chrono>

int main()
{
    using std::chrono::high_resolution_clock;
    high_resolution_clock::time_point start, end;
    using std::chrono::nanoseconds;
    using std::chrono::duration_cast;
    auto time_span1 = 0ULL, time_span2 = 0ULL;
    auto nbTests = 0U;
    rc::check("Append same character until a specific position", [&]() 
    {        
        auto c = *rc::gen::suchThat<char>([](char c) { return c > 31 && c < 127; });
        auto pos = *rc::gen::inRange<unsigned int>(1, StrStream::capacity()-1);

        start = high_resolution_clock::now();
        StrStream strstream;
        strstream.append(pos, c);
        end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        std::string str(pos, c);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
        
        RC_ASSERT(strstream.c_str() == str);
    });
    if (nbTests)
    {
        std::cout << "Append a character up to a specific position perfs [" << time_span1/nbTests 
            << "] std::string [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Append a string to a string (< 1024 characters)", [&](std::string strOrigin, std::string strToAppend) 
    {
        auto len = strOrigin.length() + strToAppend.length();
        while (len > FiniteStr<>::capacity())
        {
            strOrigin = *rc::gen::arbitrary<std::string>();
            strToAppend = *rc::gen::arbitrary<std::string>();
            len = strOrigin.length() + strToAppend.length();
        }
        
        start = high_resolution_clock::now();
        StrStream strstream;
        strstream.append(strOrigin.c_str(), strOrigin.length());
        strstream.append(strToAppend.c_str(), strToAppend.length());
        end = high_resolution_clock::now();
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
        std::cout << "Append a string to a string (< 1024 characters) perfs [" << time_span1/nbTests 
            << "] std::string [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Consider sputn instead of operator<<", [&](std::string str) 
    {
        std::ofstream null("/dev/null");
        null.sync_with_stdio(false);
        null << str; // dry run
        null.flush();

        start = high_resolution_clock::now();
        null.rdbuf()->sputn(str.c_str(), str.length());
        end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        null.flush();

        start = high_resolution_clock::now();
        null << str;
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        null.flush();
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Consider sputn perfs  [" << time_span1/nbTests 
            << "] instead of operator<< [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Consider no flush at each output", [&](std::string str) 
    {        
        std::ofstream null("/dev/null");
        null.sync_with_stdio(false);
        start = high_resolution_clock::now();
        null.rdbuf()->sputn(str.c_str(), str.length());
        null.flush();
        end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "If we flush at each output, sputn perfs : [" 
            << time_span1/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Append a string to a string (> 1024 characters)", [&]() 
    {
        auto size = *rc::gen::inRange<int>(1, StrStream::capacity());
        std::string strOrigin = *rc::gen::container<std::string>(size, rc::gen::character<typename std::string::value_type>());
        size = *rc::gen::inRange<int>(StrStream::capacity()+1, 2*StrStream::capacity());
        auto strToAppend = *rc::gen::container<std::string>(size, rc::gen::character<typename std::string::value_type>());
    
        start = high_resolution_clock::now();
        StrStream strstream;
        strstream.append(strOrigin.c_str(), strOrigin.length());
        strstream.append(strToAppend.c_str(), strToAppend.length());
        end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        std::string str(strOrigin);
        str += strToAppend;
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        RC_LOG() /*std::cout*/
            << "Append a string to a string (> 1024 characters) strOrigin length [" << strOrigin.length() 
            << "] strToAppend length [" << strToAppend.length() << "] strstream length [" << strstream.length()
            << "] string length [" << str.length() << "]" << std::endl;
        
        ++nbTests;
        
        RC_ASSERT(strstream.c_str() == str);
    });
    if (nbTests)
    {
        std::cout << "Append a string to a string (> 1024 characters) perfs [" << time_span1/nbTests 
            << "] std::string [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Append an unsigned int to a string (< 1024 characters)", [&](std::string strOrigin, const unsigned int intToAppend)
    {
        auto len = strOrigin.length();
        while (len > FiniteStr<>::capacity() - 64)
        {
            strOrigin = *rc::gen::arbitrary<std::string>();
            len = strOrigin.length();
        }
        
        start = high_resolution_clock::now();
        StrStream strstream;
        strstream.append(strOrigin.c_str(), strOrigin.length());
        strstream << intToAppend;
        end = high_resolution_clock::now();
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
        std::cout << "Append an unsigned int to a string (< 1024 characters) perfs [" << time_span1/nbTests 
            << "] std::stringstream [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Append a double to a string (< 1024 characters)", [&](std::string strOrigin) 
    {
        const auto doubleToAppend = *rc::gen::positive<double>();
                
        auto len = strOrigin.length();
        while (len > FiniteStr<>::capacity() - 64)
        {
            strOrigin = *rc::gen::arbitrary<std::string>();
            len = strOrigin.length();
        }
        
        start = high_resolution_clock::now();
        StrStream strstream;
        strstream.append(strOrigin.c_str(), strOrigin.length());
        strstream << doubleToAppend;
        end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        std::stringstream sstr;
        sstr << std::fixed /*<< std::setprecision(std::numeric_limits<double>::digits10)*/ << strOrigin;
        sstr << doubleToAppend;
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "Append a double to a string (< 1024 characters) doubleToAppend [" << doubleToAppend 
            << "] strOrigin [" << strOrigin << "] strstream [" << strstream << "] stringstream [" << sstr.str()
            << "]" << std::endl;
        
        ++nbTests;
        
        const std::string cmp1(strstream.c_str(), strstream.length()-1);
        const std::string cmp2(sstr.str().c_str(), sstr.str().length()-1);
        RC_ASSERT(cmp1 == cmp2);
    });
    if (nbTests)
    {
        std::cout << "Append a double to a string (< 1024 characters) perfs [" << time_span1/nbTests 
            << "] std::stringstream [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
}

