#pragma once

#include "Common.h"

#include <tuple>
#include <map>
#include <deque>

using namespace common;

class FeedHandler
{    
public:
    FeedHandler() {}

    bool processMessage(const std::string& line);
    void printCurrentOrderBook(std::ostream& os) const {}
    
private:
    using Limit = std::tuple<Quantity, Price>;
    std::deque<Limit> buys, sells;
    
    using Order = std::tuple<LimitNum, char, Quantity, Price>;
    std::map<OrderId, Order> orders;
};



