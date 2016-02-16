#include "FeedHandler.h"
#include "Reporter.h"
#include <utils/SimpleBuffer.h>

#include <cstring>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <thread>
#include <chrono>

int main(int argc, char **argv)
{
    if (argc < 2 || !strcmp(argv[1], "-h"))
    {
        std::cerr << "Usage:\t<program name> <file> [-v <verbose>]" << std::endl;
        return -1;
    }
    
    auto verbose = 0;
    if (argc == 4)
    {
        if (!strcmp(argv[2], "-v")) verbose = std::stoi(argv[3]);
    }
    std::cout << "Verbose is " << verbose << " : default is 0, param '-v 1 or higher' to activate it" << std::endl;
    std::cout.sync_with_stdio(false);
    std::cerr.sync_with_stdio(false);
    
    const std::string filename(argv[1]);
    int fd = open(filename.c_str(), O_RDONLY, 0);
    if (-1 == fd)
    {
        std::cerr << "Expected a file (see usage) or [" << filename << "] not readable!" << std::endl;
        return -1;
    }
    auto getFilesize = [](const std::string& filename)
    {
        struct stat st;
        stat(filename.c_str(), &st);
        return st.st_size;
    };
    size_t filesize = getFilesize(filename);
    
//    mlockall(MCL_FUTURE);

    using std::chrono::high_resolution_clock;
    high_resolution_clock::time_point start = high_resolution_clock::now();
    
    void* mmappedData = mmap(0, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    if (unlikely(mmappedData == MAP_FAILED))
    {
        std::cerr << "Unable to mmap file [" << filename << "]!" << std::endl;
        return -1;
    }
    SimpleBuffer sbuffer(static_cast<char*>(mmappedData), filesize);
    sbuffer.seekEnd(filesize);
//    mlockall(MCL_CURRENT|MCL_FUTURE);
//    mlock(mmappedData, filesize);
    
    WaitFreeQueue<FeedHandler::Data> queue;
    FeedHandler feed(queue);
    Reporter reporter;
    Errors errors;
    
    auto threaded_reporter = [&]() 
    {
        auto counter = 0;
        while(1)
        {
            if (likely(reporter.processData(queue.pop_front())))
            {
                ++counter;
                if (counter > 10)
                {
                    reporter.printCurrentOrderBook(std::cerr);
                    counter = 0;
                }
                reporter.printMidQuotesAndTrades(std::cerr, errors);
            }
            else break;
        }
    };
    std::thread thr(threaded_reporter);
    
    high_resolution_clock::time_point start2 = high_resolution_clock::now();
    
    while(sbuffer.available())
    {
        auto pos = sbuffer.getPosition('\n');
        if (unlikely(pos < 0)) break;
        feed.processMessage(static_cast<const char*>(&sbuffer[0]), pos, errors, verbose);
        sbuffer.seek(pos+1);
    }
    high_resolution_clock::time_point end2 = high_resolution_clock::now();
    queue.dontSpin();
    
    thr.join();
    
    reporter.printCurrentOrderBook(std::cout);
    reporter.printErrors(std::cout, errors, verbose);
        
    high_resolution_clock::time_point end = high_resolution_clock::now();
    using std::chrono::seconds;
    using std::chrono::microseconds;
    using std::chrono::duration;
    using std::chrono::duration_cast;
    auto howlong2 = duration<double>(end2 - start2);
    auto sec2 = duration_cast<seconds>(howlong2).count();
    auto usec2 = duration_cast<microseconds>(howlong2).count();
    
    auto howlong = duration<double>(end - start);
    auto sec = duration_cast<seconds>(howlong).count();
    auto usec = duration_cast<microseconds>(howlong).count();
    
    std::cout << "Overall run perfs: " << sec << " sec " << usec % 1'000'000 
        << " usec (building OB: " << sec2 << " sec " << usec2  % 1'000'000 << " usec)"
        << std::endl;
        
    munmap(mmappedData, filesize);
    close(fd);
    return 0;
}

