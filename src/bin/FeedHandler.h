#pragma once

#include "Common.h"

#include <tuple>
#include <unordered_map>
#include <deque>

using namespace common;

class FeedHandler
{    
public:
    FeedHandler() {}

    bool processMessage(const std::string& line);
    void printCurrentOrderBook(std::ostream& os) const {}
    
private:
    using Order = std::tuple<Quantity, Price>;
    using Limit = Order;
    static Quantity& getQty(Order& order) { return std::get<0>(order); }
    static Price& getPrice(Order& order) { return std::get<1>(order); }
    
    bool NewBuyOrder(OrderId orderId, Order&& order);
    bool NewSellOrder(OrderId orderId, Order&& order);
    
    bool CancelBuyOrder(OrderId orderId, Order&& order);
    bool CancelSellOrder(OrderId orderId, Order&& order);
    
    bool ModifyBuyOrder(OrderId orderId, Order&& order);
    bool ModifySellOrder(OrderId orderId, Order&& order);
    
private:
    std::deque<Limit> bids_, asks_;
    std::unordered_map<OrderId, Order> buyOrders_, sellOrders_;
};



