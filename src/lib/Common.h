#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

namespace common
{
    using OrderId = unsigned int;
    using Quantity = unsigned int;
    using Price = double;
    
    struct Errors
    {
        // Parsing
        unsigned long long commentedLines = 0;
        unsigned long long blankLines = 0;
        
        unsigned long long corruptedMessages = 0;
                
        unsigned long long negativeOrderIds = 0;
        unsigned long long negativeQuantities = 0;
        unsigned long long negativePrices = 0;
        
        unsigned long long missingOrderIds = 0;
        unsigned long long missingQuantities = 0;
        unsigned long long missingPrices = 0;
        
        unsigned long long outOfBoundsOrderIds = 0;
        unsigned long long outOfBoundsQuantities = 0;
        unsigned long long outOfBoundsPrices = 0;
        
        // Order Management
        unsigned long long duplicateOrderIds = 0;
        unsigned long long modifiesWithUnknownOrderId = 0;
        unsigned long long cancelsWithUnknownOrderId = 0;
        unsigned long long bestBidEqualOrUpperThanBestAsk = 0;
        
        // Trade Management
        unsigned long long tradesWithUnknownOrderId = 0;
    };
}

