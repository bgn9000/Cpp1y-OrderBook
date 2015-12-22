#include "FeedHandler.h"

#include "Parser.h"

bool FeedHandler::processMessage(const std::string& line)
{
//    std::cout << line << std::endl;  
    Parser p;
    if (!p.parse(line)) return false;
    
    
    return true;
}

