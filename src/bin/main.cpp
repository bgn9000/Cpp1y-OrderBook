#include "FeedHandler.h"

int main(int argc, char **argv)
{
    FeedHandler feed;
    
    std::string line;
    const std::string filename(argv[1]);
    std::ifstream infile(filename.c_str(), std::ios::in);
    
    int counter = 0;
    while (std::getline(infile, line)) 
    {
        if (feed.processMessage(line) && ++counter % 10 == 0) 
        {
            feed.printCurrentOrderBook(std::cerr);
        }
        feed.printMidQuotes(std::cout);
    }
    feed.printCurrentOrderBook(std::cout);
    return 0;
}
