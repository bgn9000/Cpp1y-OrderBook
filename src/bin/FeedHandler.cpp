#include "FeedHandler.h"

#include "Parser.h"

bool FeedHandler::processMessage(const std::string& line)
{
//    std::cout << line << std::endl;  
    Parser p;
    if (!p.parse(line)) return false;
    
    
    return true;
}

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
    }
    feed.printCurrentOrderBook(std::cout);
    return 0;
}
