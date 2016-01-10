#include "FeedHandler.h"
#include "SimpleBuffer.h"

#include <cstring>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <chrono>
using namespace std::chrono;

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
    
    FeedHandler feed;
    Errors errors;
    int counter = 0;
    while(sbuffer.available())
    {
        auto pos = sbuffer.getPosition('\n');
        if (unlikely(pos < 0)) break;
        if (likely(feed.processMessage(static_cast<const char*>(&sbuffer[0]), pos, errors, verbose)))
        {
            ++counter;
            if (counter > 10)
            {
                feed.printCurrentOrderBook(std::cerr);
                counter = 0;
            }
            feed.printMidQuotesAndTrades(std::cerr, errors);
        }
        sbuffer.seek(pos+1);        
    }    
    feed.printCurrentOrderBook(std::cout);
    feed.printErrors(std::cout, errors, verbose);
        
    high_resolution_clock::time_point end = high_resolution_clock::now();
    auto howlong = duration<double>(end - start);
    std::cout << "Overall run perfs : " << duration_cast<seconds>(howlong).count() << " sec "
        << duration_cast<microseconds>(howlong).count() << " usec " << std::endl;
        
    munmap(mmappedData, filesize);
    close(fd);
    return 0;
}

