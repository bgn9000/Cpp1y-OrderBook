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
    void printMidQuotes(std::ostream& os) const;
    void printCurrentOrderBook(std::ostream& os) const {}
  
    using Order = std::tuple<Quantity, Price>;
    using Limit = Order;
    static Quantity& getQty(Order& order) { return std::get<0>(order); }
    static Price& getPrice(Order& order) { return std::get<1>(order); }
    static Quantity getQty(const Order& order) { return std::get<0>(order); }
    static Price getPrice(const Order& order) { return std::get<1>(order); }
    
protected:
    bool newBuyOrder(OrderId orderId, Order&& order, const int verbose = 0);
    bool newSellOrder(OrderId orderId, Order&& order, const int verbose = 0);
    
    bool cancelBuyOrder(OrderId orderId, Order&& order, const int verbose = 0);
    bool cancelSellOrder(OrderId orderId, Order&& order, const int verbose = 0);
    
    bool modifyBuyOrder(OrderId orderId, Order&& order, const int verbose = 0);
    bool modifySellOrder(OrderId orderId, Order&& order, const int verbose = 0);
    
    std::deque<Limit> bids_, asks_;
    std::unordered_map<OrderId, Order> buyOrders_, sellOrders_;
};



