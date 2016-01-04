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
    using AggregatedQty = unsigned long long;
    using Price = double;
    
    static constexpr int maxOrderId = (1'000'000'000 -1);
    static constexpr int maxOrderQty = (1'000'000 -1);
    static constexpr int maxOrderPrice = (1'000'000'000 -1);

    namespace
    {
        constexpr auto nbChar(int v)
        {
            auto nb = 0U;
            while (v > 0) { ++nb; v /= 10; }
            return nb;
        }
    }
    static constexpr int nbCharOfOrderId = nbChar(maxOrderId);
    static constexpr int nbCharOfOrderQty = nbChar(maxOrderQty);
    static constexpr int nbCharOfPrice = nbChar(maxOrderPrice);
    static constexpr int nbCharOfPricePrecision = 6;
    
    struct Errors
    {
        // Parsing
        unsigned long long commentedLines = 0;
        unsigned long long blankLines = 0;
        
        unsigned long long corruptedMessages = 0;
        unsigned long long IncompleteMessages = 0;
        unsigned long long wrongActions = 0;
        unsigned long long wrongSides = 0;
                
        unsigned long long negativeOrderIds = 0;
        unsigned long long negativeQuantities = 0;
        unsigned long long negativePrices = 0;
        
        unsigned long long missingActions = 0;
        unsigned long long missingOrderIds = 0;
        unsigned long long missingSides = 0;
        unsigned long long missingQuantities = 0;
        unsigned long long missingPrices = 0;
        
        unsigned long long zeroOrderIds = 0;
        unsigned long long zeroQuantities = 0;
        unsigned long long zeroPrices = 0;
        
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
        
        unsigned long long nbErrors()
        {
            return  corruptedMessages +
                    IncompleteMessages +
                    wrongActions +
                    wrongSides +
                    negativeOrderIds +
                    negativeQuantities +
                    negativePrices +
                    missingActions +
                    missingOrderIds +
                    missingSides +
                    missingQuantities +
                    missingPrices +
                    zeroOrderIds +
                    zeroQuantities +
                    zeroPrices +
                    outOfBoundsOrderIds +
                    outOfBoundsQuantities +
                    outOfBoundsPrices +
                    duplicateOrderIds +
                    modifiesWithUnknownOrderId +
                    cancelsWithUnknownOrderId +
                    bestBidEqualOrUpperThanBestAsk +
                    tradesWithUnknownOrderId;
        }
    };
}

