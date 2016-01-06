#include "FeedHandler.h"

#include <cstring>

int main(int argc, char **argv)
{
    auto verbose = 0;
    if (argc == 4)
    {
        if (!strcmp(argv[2], "-v")) verbose = std::stoi(argv[3]);
    }
    std::cout << "Verbose is " << verbose << " : default is 0, param '-v 1 or higher' to activate it" << std::endl;
    std::cout.sync_with_stdio(false);
    std::cerr.sync_with_stdio(false);
    
    FeedHandler feed;
    
    std::string line;
    const std::string filename(argv[1]);
    std::ifstream infile(filename.c_str(), std::ios::in);
    
    Errors errors;
    
    int counter = 0;
    while (std::getline(infile, line)) 
    {
        if (feed.processMessage(line, errors, verbose) && ++counter % 10 == 0) 
        {
            feed.printCurrentOrderBook(std::cerr);
        }
        feed.printMidQuotes(std::cerr);
    }
    feed.printCurrentOrderBook(std::cout);
    feed.printErrors(std::cout, errors, verbose);
    return 0;
}
