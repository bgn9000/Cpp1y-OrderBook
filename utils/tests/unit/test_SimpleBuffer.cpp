#include <rapidcheck.h>

#include "utils/SimpleBuffer.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <chrono>
using namespace std::chrono;

int main()
{
    auto time_span1 = 0ULL, time_span2 = 0ULL;
    auto nbTests = 0U;
    rc::check("Append string", [&](std::string strOrigin, std::string strToAppend) 
    {
        auto len = strOrigin.length() + strToAppend.length();
        while (len > SimpleBuffer::SIMPLE_BUFFER_SIZE)
        {
            strOrigin = *rc::gen::arbitrary<std::string>();
            strToAppend = *rc::gen::arbitrary<std::string>();
            len = strOrigin.length() + strToAppend.length();
        }
        
        high_resolution_clock::time_point start = high_resolution_clock::now();
        SimpleBuffer sbuffer(len);
        sbuffer.push(strOrigin.c_str(), strOrigin.length());
        sbuffer.push(strToAppend.c_str(), strToAppend.length());
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        std::string str(strOrigin);
        str += strToAppend;
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
        
        RC_ASSERT(sbuffer.available() == str.length());
        std::string str2(static_cast<const char*>(&sbuffer[0]), sbuffer.available());
        RC_ASSERT(str2 == str);
    });
    if (nbTests)
    {
        std::cout << "Append a string to a string perfs [" << time_span1/nbTests 
            << "] std::string [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Get character position into a string", [&]() 
    {
        auto c = *rc::gen::arbitrary<char>();
        auto str = *rc::gen::container<std::string>(rc::gen::distinctFrom(rc::gen::character<typename std::string::value_type>(), c));
        str += str; // longer string
        str += c;
        SimpleBuffer sbuffer;
        sbuffer.push(str.c_str(), str.length());
        
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto pos1 = sbuffer.getPosition(c);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        auto pos2 = str.find_first_of(c);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
        
        RC_ASSERT(sbuffer.available() == str.length());
        std::string str2(static_cast<const char*>(&sbuffer[0]), sbuffer.available());
        RC_ASSERT(str2 == str);
        RC_ASSERT(pos1 == static_cast<int>(pos2));
    });
    if (nbTests)
    {
        std::cout << "Get character position into a string perfs [" << time_span1/nbTests 
            << "] std::string::find_first_of [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Push on left", [&]()
    {
        auto c = *rc::gen::arbitrary<char>();
        auto str = *rc::gen::container<std::string>(rc::gen::distinctFrom(rc::gen::character<typename std::string::value_type>(), c));
        str += str; // longer string
        str += c;
        SimpleBuffer sbuffer;
        sbuffer.push(str.c_str(), str.length());
        
        auto pos1 = sbuffer.getPosition(c);
        auto pos2 = str.find_first_of(c);
        
        high_resolution_clock::time_point start = high_resolution_clock::now();
        sbuffer.seek(pos1);
        sbuffer.pushOnLeft();
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        start = high_resolution_clock::now();
        str.erase(str.begin(), str.begin()+pos2);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
        
        RC_ASSERT(sbuffer.available() == str.length());
        std::string str2(static_cast<const char*>(&sbuffer[0]), sbuffer.available());
        RC_ASSERT(str2 == str);
        RC_ASSERT(pos1 == static_cast<int>(pos2));
    });
    if (nbTests)
    {
        std::cout << "Push on left [" << time_span1/nbTests 
            << "] std::string::erase [" << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    auto time_span3 = 0ULL;
    rc::check("Read files line by line", [&]()
    {
        std::string filename("../../tests/test6.txt");
        std::ifstream infile1(filename, std::ios::in);
        infile1.sync_with_stdio(false);
        SimpleBuffer sbuffer;
        auto nbLignes1 = 0;
        high_resolution_clock::time_point start = high_resolution_clock::now();
        while (infile1.good())
        {
            infile1.read(sbuffer.dataEnd(), sbuffer.freeSpace());
            sbuffer.seekEnd(infile1.gcount());
            if (unlikely(sbuffer.freeSpace() < 1024))
                sbuffer.pushOnLeft();
            while(sbuffer.available())
            {
                auto pos = sbuffer.getPosition('\n');
                if (pos < 0) break;
                ++nbLignes1;
                sbuffer.seek(pos+1);
            }
        }
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        std::ifstream infile2(filename, std::ios::in);
        infile2.sync_with_stdio(false);
        std::string line;        
        auto nbLignes2 = 0;
        start = high_resolution_clock::now();
        while (std::getline(infile2, line)) ++nbLignes2;
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        
        int fd = open(filename.c_str(), O_RDONLY, 0);
        auto getFilesize = [&]()
        {
            struct stat st;
            stat(filename.c_str(), &st);
            return st.st_size;
        };
        auto nbLignes3 = 0;
        start = high_resolution_clock::now();
        size_t filesize = getFilesize();
        void* mmappedData = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
        if (likely(mmappedData != MAP_FAILED))
        {
            SimpleBuffer sbuffer2(static_cast<char*>(mmappedData), filesize);
            sbuffer2.seekEnd(filesize);
            while(sbuffer2.available())
            {
                auto pos = sbuffer2.getPosition('\n');
                if (pos < 0) break;
                ++nbLignes3;
                sbuffer2.seek(pos+1);
            }
            munmap(mmappedData, filesize);
        }
        end = high_resolution_clock::now();
        time_span3 += duration_cast<nanoseconds>(end - start).count();
        close(fd);
        
        ++nbTests;
        
        RC_ASSERT(nbLignes1 == nbLignes2);
        RC_ASSERT(nbLignes1 == nbLignes3);
    });
    if (nbTests)
    {
        std::cout << "Read files line by line [" << time_span1/nbTests 
            << "] std::getline [" << time_span2/nbTests
            << "] with mmap [" << time_span3/nbTests  
            << "] (in ns)" << std::endl;
    }
    
    return 0;
}
