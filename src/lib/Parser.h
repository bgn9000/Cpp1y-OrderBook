#pragma once

#include "Common.h"

using namespace common;

class Parser
{
public:
    enum class Action : char
    {
        ADD = 'A',
        CANCEL = 'X',
        MODIFY = 'M',
        TRADE = 'T',
    };
    
    enum class Side : char
    {
        BUY = 'B',
        SELL = 'S',
    };
    
public:
    bool parse(const std::string& str, const int verbose = 0);
    
    char getAction() { return action_; }
    OrderId getOrderId() { return orderId_; }
    char getSide() { return side_; }
    Price getPrice() { return price_; }
    Quantity getQty() { return qty_; }
    
private:
    char action_ = 0;
    OrderId orderId_ = 0;
    char side_ = 0;
    Price price_ = 0.0;
    Quantity qty_ = 0;
};

