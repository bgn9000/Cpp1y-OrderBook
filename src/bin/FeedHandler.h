#pragma once

#include "Common.h"

class FeedHandler
{

    
public:
    FeedHandler() {}

    bool processMessage(const std::string& line);
    void printCurrentOrderBook(std::ostream& os) const {}
};



