#pragma once

#include "Common.h"
#include "CircularBlock.h"

#include <unordered_map>
#include <deque>

using namespace common;

class FeedHandler
{
public:
    struct Data
    {
        Data() = default;
        Data(char action, char side, unsigned int pos, Limit limit = Limit{0, 0.0})
            : action_(action), side_(side), pos_(pos), limit_(limit)
        {
        }
        Data(const Data& data) = default;
        Data& operator=(const Data& data) = delete;
        Data(Data&& data) = default;
        Data& operator=(Data&& data) = default;
        
        char pad1_[64] = "";
        char action_ = 0;
        char side_ = 0;
        unsigned int pos_ = 0;
        Limit limit_{0, 0.0};
        char pad2_[64] = "";
    };
    
    FeedHandler(CircularBlock<Data>& block) : block_(block) {}
    ~FeedHandler() = default;
    FeedHandler(const FeedHandler&) = delete;
    FeedHandler& operator=(const FeedHandler&) = delete;

    void processMessage(const char* data, size_t dataLen, Errors& errors, const int verbose = 0);
        
protected:
    void newBuyOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0);
    void newSellOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0);
    
    void cancelBuyOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0);
    void cancelSellOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0);
    
    void modifyBuyOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0);
    void modifySellOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0);
    
    std::deque<Limit> bids_, asks_;
    std::unordered_map<OrderId, Order> buyOrders_, sellOrders_;
    
    CircularBlock<Data>& block_;
};

