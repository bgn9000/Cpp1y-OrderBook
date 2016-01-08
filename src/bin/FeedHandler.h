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

    bool processMessage(const char* data, size_t dataLen, Errors& errors, const int verbose = 0);

    void printCurrentOrderBook(std::ostream& os) const;
    void printMidQuotesAndTrades(std::ostream& os);
    void printErrors(std::ostream& os, Errors& errors, const int verbose = 0);
  
    using Order = std::tuple<Quantity, Price>;
    static Quantity& getQty(Order& order) { return std::get<0>(order); }
    static Price& getPrice(Order& order) { return std::get<1>(order); }
    static Quantity getQty(const Order& order) { return std::get<0>(order); }
    static Price getPrice(const Order& order) { return std::get<1>(order); }
    
    using Limit = std::tuple<AggregatedQty, Price>;
    static AggregatedQty& getQty(Limit& limit) { return std::get<0>(limit); }
    static Price& getPrice(Limit& limit) { return std::get<1>(limit); }
    static AggregatedQty getQty(const Limit& limit) { return std::get<0>(limit); }
    static Price getPrice(const Limit& limit) { return std::get<1>(limit); }
    using Trade = std::tuple<AggregatedQty, Price>;
    
protected:
    bool newBuyOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0);
    bool newSellOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0);
    
    bool cancelBuyOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0);
    bool cancelSellOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0);
    
    bool modifyBuyOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0);
    bool modifySellOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0);
    
    bool treatTrade(Trade&& newTrade);
    
    std::deque<Limit> bids_, asks_;
    std::unordered_map<OrderId, Order> buyOrders_, sellOrders_;
    Trade currentTrade{0ULL, 0.0};
    bool receivedNewTrade = false;
};



