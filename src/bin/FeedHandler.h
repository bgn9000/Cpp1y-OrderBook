#pragma once

#include "Common.h"

#include <tuple>
#include <unordered_map>
#include <deque>

using namespace common;

class FeedHandler
{    
public:
    FeedHandler() = default;
    ~FeedHandler() = default;
    FeedHandler(const FeedHandler&) = delete;
    FeedHandler& operator=(const FeedHandler&) = delete;

    bool processMessage(const std::string& line);
    void printCurrentOrderBook(std::ostream& os) const {}
  
    using Order = std::tuple<Quantity, Price>;
    using Limit = Order;
    static Quantity& getQty(Order& order) { return std::get<0>(order); }
    static Price& getPrice(Order& order) { return std::get<1>(order); }
    
    bool NewBuyOrder(OrderId orderId, Order&& order, const int verbose = 0);
    bool NewSellOrder(OrderId orderId, Order&& order, const int verbose = 0);
    
    bool CancelBuyOrder(OrderId orderId, Order&& order, const int verbose = 0);
    bool CancelSellOrder(OrderId orderId, Order&& order, const int verbose = 0);
    
    bool ModifyBuyOrder(OrderId orderId, Order&& order, const int verbose = 0);
    bool ModifySellOrder(OrderId orderId, Order&& order, const int verbose = 0);
  
protected:
    std::deque<Limit> bids_, asks_;
    std::unordered_map<OrderId, Order> buyOrders_, sellOrders_;
};



