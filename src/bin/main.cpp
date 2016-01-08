#include "FeedHandler.h"
#include "SimpleBuffer.h"

#include <cstring>

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
    std::ifstream infile(filename.c_str(), std::ios::in);
    if (!infile.is_open())
    {
        std::cerr << "Expected a file (see usage) or [" << filename << "] not readable!" << std::endl;
        return -1;
    }
    infile.sync_with_stdio(false);
    
    high_resolution_clock::time_point start = high_resolution_clock::now();

    FeedHandler feed;
    SimpleBuffer sbuffer;
    Errors errors;
    int counter = 0;
    while (infile.good())
    {
        infile.read(sbuffer.dataEnd(), sbuffer.freeSpace());
        sbuffer.seekEnd(infile.gcount());
        if (unlikely(sbuffer.freeSpace() < 1024))
        {
            sbuffer.pushOnLeft();
            if (unlikely(sbuffer.freeSpace() < 1024))
            {
                std::cerr << "Bad input file: line too long to process!" << std::endl;
                break;
            }
        }
        while(sbuffer.available())
        {
            auto pos = sbuffer.getPosition('\n');
            if (pos < 0) break;
            if (likely(feed.processMessage(static_cast<const char*>(&sbuffer[0]), pos, errors, verbose)))
            {
                ++counter;
                if (counter > 10)
                {
                    feed.printCurrentOrderBook(std::cerr);
                    counter = 0;
                }
                feed.printMidQuotesAndTrades(std::cerr);
            }
            sbuffer.seek(pos+1);
        }
    }
    feed.printCurrentOrderBook(std::cout);
    feed.printErrors(std::cout, errors, verbose);
    
    high_resolution_clock::time_point end = high_resolution_clock::now();
    auto howlong = duration<double>(end - start);
    std::cout << "Overall run perfs : " << duration_cast<seconds>(howlong).count() << " sec "
        << duration_cast<microseconds>(howlong).count() << " usec " << std::endl;
    return 0;
}
