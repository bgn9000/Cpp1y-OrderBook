#pragma once

#include "Common.h"

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
    unsigned int getOrderId() { return orderId_; }
    char getSide() { return side_; }
    double getPrice() { return price_; }
    double getQty() { return qty_; }
    
private:
    char action_ = 0;
    unsigned int orderId_ = 0;
    char side_ = 0;
    double price_ = 0.0;
    unsigned int qty_ = 0;
};

