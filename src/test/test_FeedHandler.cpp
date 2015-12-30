//#include <gtest/gtest.h>
#include <rapidcheck.h>

#include "FeedHandler.h"

#include <cmath>
#include <cstring>
#include <iterator>
#include <set>

#include <chrono>
using namespace common;
using namespace std::chrono;

class rcFeedHandler : public FeedHandler
{
public:
    auto getNbBuyOrders() { return buyOrders_.size(); }
    auto getNbSellOrders() { return sellOrders_.size(); }
    auto getNbBids() { return bids_.size(); }
    auto getNbAsks() { return asks_.size(); }
    
    inline bool newBuyOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0)
    { return FeedHandler::newBuyOrder(orderId, std::forward<Order>(order), errors, verbose); }
    inline bool newSellOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0)
    { return FeedHandler::newSellOrder(orderId, std::forward<Order>(order), errors, verbose); }
    
    inline bool cancelBuyOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0)
    { return FeedHandler::cancelBuyOrder(orderId, std::forward<Order>(order), errors, verbose); }
    inline bool cancelSellOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0)
    { return FeedHandler::cancelSellOrder(orderId, std::forward<Order>(order), errors, verbose); }
    
    inline bool modifyBuyOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0)
    { return FeedHandler::modifyBuyOrder(orderId, std::forward<Order>(order), errors, verbose); }
    inline bool modifySellOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose = 0)
    { return FeedHandler::modifySellOrder(orderId, std::forward<Order>(order), errors, verbose); }
    
    std::map<OrderId, Order> buyOrders, sellOrders;    
    std::map<Price, int> uniqueBidPrices, uniqueAskPrices;
};

int main(int argc, char **argv)
{
    auto verbose = 0;
    if (argc == 3)
    {
        if (!strcmp(argv[1], "-v")) verbose = std::stoi(argv[2]);
    }
    std::cout << "Verbose is " << verbose << " : default is 0, param '-v 1 or higher' to activate it" << std::endl;
    
#if 0
    std::deque<int> deque = { 2, 4, 5, 7, 8, 9 };
    auto lower = std::lower_bound(deque.begin(), deque.end(), 1,
        [](int& i, int j) -> bool
        {
            return (i < j);
        });
    if (lower == deque.begin())
    {
        std::cout << "S 1 : begin : " << *lower << std::endl;
        deque.insert(lower, 1);
    }
    lower = std::lower_bound(deque.begin(), deque.end(), 6,
        [](int& i, int j) -> bool
        {
            return (i < j);
        });
    std::cout << "S 6 : " << *lower << std::endl;
    deque.insert(lower, 6);
    lower = std::lower_bound(deque.begin(), deque.end(), 10,
        [](int& i, int j) -> bool
        {
            return (i < j);
        });
    if (lower == deque.end())
    {
        std::cout << "S 10 : end" << std::endl;
        deque.push_back(10);
    }
    std::copy(deque.begin(), deque.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
#endif
#if 0
    std::deque<int> deque = { 9, 8, 7, 5, 4, 2 };
    auto lower = std::lower_bound(deque.begin(), deque.end(), 10,
        [](int& i, int j) -> bool
        {
            return (i > j);
        });
    if (lower == deque.begin())
    {
        std::cout << "S 10 : begin : " << *lower << std::endl;
        deque.insert(lower, 10);
    }
    lower = std::lower_bound(deque.begin(), deque.end(), 6,
        [](int& i, int j) -> bool
        {
            return (i > j);
        });
    std::cout << "S 6 : " << *lower << std::endl;
    deque.insert(lower, 6);
    lower = std::lower_bound(deque.begin(), deque.end(), 1,
        [](int& i, int j) -> bool
        {
            return (i > j);
        });
    if (lower == deque.end())
    {
        std::cout << "S 1 : end" << std::endl;
        deque.push_back(1);
    }
    std::copy(deque.begin(), deque.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
#endif    
#if 1
    auto time_span1 = 0ULL;
    auto nbTests = 0U;
    rcFeedHandler FH;
    rc::check("New Buy Orders", [&]()
    {
        Price price = *rc::gen::positive<Price>();
        while (FH.uniqueBidPrices.find(price) != FH.uniqueBidPrices.end())
        { price = *rc::gen::positive<Price>(); }
        FH.uniqueBidPrices[price] = 1;
        
        Quantity qty = *rc::gen::inRange(10, 1000);
        
        auto orderId = *rc::gen::suchThat(rc::gen::positive<OrderId>(), [](int x) { return (x % 2) == 0; });
        while (FH.buyOrders.find(orderId) != FH.buyOrders.end())
        { 
            orderId = *rc::gen::suchThat(rc::gen::positive<OrderId>(), [](int x) { return (x % 2) == 0; });
        }
        FH.buyOrders[orderId] = FeedHandler::Order{qty, price};
        
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "BUY orderId [" << orderId << "] qty [" << qty << "] price [" << price << "]" << std::endl;
        
        Errors errors;
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto ret = FH.newBuyOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        RC_ASSERT(true == ret);
        RC_ASSERT(FH.newSellOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose) == false);
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbSellOrders() == 0U);
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        RC_ASSERT(FH.getNbAsks() == 0U);
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "New Buy Orders perfs : [" << time_span1/nbTests << "] (in ns)" << std::endl;
    }

    time_span1 = 0ULL;
    nbTests = 0U;
    rc::check("New Sell Orders", [&]()
    {
        Price price = *rc::gen::positive<Price>();
        while (FH.uniqueAskPrices.find(price) != FH.uniqueAskPrices.end())
        { price = *rc::gen::positive<Price>(); }
        FH.uniqueAskPrices[price] = 1;
        
        Quantity qty = *rc::gen::inRange(10, 1000);
        
        auto orderId = *rc::gen::suchThat(rc::gen::positive<OrderId>(), [](int x) { return (x % 2) != 0; });
        while (FH.sellOrders.find(orderId) != FH.sellOrders.end())
        { 
            orderId = *rc::gen::suchThat(rc::gen::positive<OrderId>(), [](int x) { return (x % 2) != 0; });
        }
        FH.sellOrders[orderId] = FeedHandler::Order{qty, price};
        
        RC_LOG() << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "SELL orderId [" << orderId << "] qty [" << qty << "] price [" << price << "]" << std::endl;
        
        Errors errors;
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto ret = FH.newSellOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        RC_ASSERT(true == ret);
        RC_ASSERT(FH.newSellOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose) == false);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "New Sell Orders perfs : [" << time_span1/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL;
    nbTests = 0U;
    std::map<OrderId, FeedHandler::Order> buyOrders_copie = FH.buyOrders;
    auto buyOrderIt = buyOrders_copie.begin();
    rc::check("Add New Buy Orders with same price", [&]()
    {
        const auto price = FeedHandler::getPrice(buyOrderIt->second);
        FH.uniqueBidPrices[price] += 1;
        ++buyOrderIt;
        
        Quantity qty = *rc::gen::inRange(10, 1000);
        
        auto orderId = *rc::gen::suchThat(rc::gen::positive<OrderId>(), [](int x) { return (x > 1000) && (x % 2) == 0; });
        while (FH.buyOrders.find(orderId) != FH.buyOrders.end())
        { 
            orderId = *rc::gen::suchThat(rc::gen::positive<OrderId>(), [](int x) { return (x > 1000) && (x % 2) == 0; });
        }
        FH.buyOrders[orderId] = FeedHandler::Order{qty, price};
        
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "Add New BUY orderId [" << orderId << "] qty [" << qty << "] same price [" << price << "]" << std::endl;
        
        Errors errors;
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto ret = FH.newBuyOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        RC_ASSERT(true == ret);
        RC_ASSERT(FH.newSellOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose) == false);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Add New Buy Orders with same price perfs : [" << time_span1/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL;
    nbTests = 0U;
    std::map<OrderId, FeedHandler::Order> sellOrders_copie = FH.sellOrders;
    auto sellOrderIt = sellOrders_copie.begin();
    rc::check("Add New Sell Orders with same price", [&]()
    {
        const auto price = FeedHandler::getPrice(sellOrderIt->second);
        FH.uniqueAskPrices[price] += 1;
        ++sellOrderIt;
        
        Quantity qty = *rc::gen::inRange(10, 1000);
       
        auto orderId = *rc::gen::suchThat(rc::gen::positive<OrderId>(), [](int x) { return (x > 1000) && (x % 2) != 0; });
        while (FH.sellOrders.find(orderId) != FH.sellOrders.end())
        {
            orderId = *rc::gen::suchThat(rc::gen::positive<OrderId>(), [](int x) { return (x > 1000) && (x % 2) != 0; });
        }
        FH.sellOrders[orderId] = FeedHandler::Order{qty, price};
        
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "Add New SELL orderId [" << orderId << "] qty [" << qty << "] same price [" << price << "]" << std::endl;
        
        Errors errors;
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto ret = FH.newSellOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        RC_ASSERT(true == ret);
        RC_ASSERT(FH.newBuyOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose) == false);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Add New Sell Orders with same price perfs : [" << time_span1/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL;
    nbTests = 0U;
    buyOrderIt = FH.buyOrders.begin();
    rc::check("Modify Buy Orders", [&]()
    {
        const auto orderId = buyOrderIt->first;
        Quantity newqty = *rc::gen::inRange(10, 1000);
        FeedHandler::getQty(buyOrderIt->second) = newqty;
        const auto price = FeedHandler::getPrice(buyOrderIt->second);
        ++buyOrderIt;
        
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "Modify BUY orderId [" << orderId << "] new qty [" << newqty << "] price [" << price << "]" << std::endl;
        
        Errors errors;
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto ret = FH.modifyBuyOrder(orderId, FeedHandler::Order{newqty, price}, errors, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        RC_ASSERT(true == ret);
        RC_ASSERT(FH.modifySellOrder(orderId, FeedHandler::Order{newqty, price}, errors, verbose) == false);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Modify Buy Orders perfs : [" << time_span1/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL;
    nbTests = 0U;
    sellOrderIt = FH.sellOrders.begin();
    rc::check("Modify Sell Orders", [&]()
    {
        const auto orderId = sellOrderIt->first;
        Quantity newqty = *rc::gen::inRange(10, 1000);
        FeedHandler::getQty(sellOrderIt->second) = newqty;
        const auto price = FeedHandler::getPrice(sellOrderIt->second);
        ++sellOrderIt;
        
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "Modify SELL orderId [" << orderId << "] new qty [" << newqty << "] price [" << price << "]" << std::endl;
        
        Errors errors;
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto ret = FH.modifySellOrder(orderId, FeedHandler::Order{newqty, price}, errors, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        RC_ASSERT(true == ret);
        RC_ASSERT(FH.modifyBuyOrder(orderId, FeedHandler::Order{newqty, price}, errors, verbose) == false);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Modify Sell Orders perfs : [" << time_span1/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL;
    nbTests = 0U;
    auto cancelBuyOrders = [&]()
    {
        buyOrderIt = FH.buyOrders.begin();
        const auto orderId = buyOrderIt->first;
        const auto qty = FeedHandler::getQty(buyOrderIt->second);
        const auto price = FeedHandler::getPrice(buyOrderIt->second);
        FH.buyOrders.erase(buyOrderIt);
        auto uniqueBidPricesIt = FH.uniqueBidPrices.find(price);
        RC_ASSERT(uniqueBidPricesIt != FH.uniqueBidPrices.end());
        if (1 == uniqueBidPricesIt->second)
        {
            FH.uniqueBidPrices.erase(uniqueBidPricesIt);
        }
        else
        {
            uniqueBidPricesIt->second -= 1;
        }
        
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "Cancel BUY orderId [" << orderId << "] qty [" << qty << "] price [" << price << "]" << std::endl;
        
        Errors errors;
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto ret = FH.cancelBuyOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        RC_ASSERT(true == ret);
        RC_ASSERT(FH.cancelSellOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose) == false);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        ++nbTests;
    };
    rc::check("Cancel Buy Orders", cancelBuyOrders);
    rc::check("Cancel Buy Orders", cancelBuyOrders);
    if (nbTests)
    {
        std::cout << "Cancel Buy Orders perfs : [" << time_span1/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL;
    nbTests = 0U;
    auto cancelSellOrders = [&]()
    {
        sellOrderIt = FH.sellOrders.begin();
        const auto orderId = sellOrderIt->first;
        const auto qty = FeedHandler::getQty(sellOrderIt->second);
        const auto price = FeedHandler::getPrice(sellOrderIt->second);
        FH.sellOrders.erase(sellOrderIt);
        auto uniqueAskPricesIt = FH.uniqueAskPrices.find(price);
        RC_ASSERT(uniqueAskPricesIt != FH.uniqueAskPrices.end());
        if (1 == uniqueAskPricesIt->second)
        {
            FH.uniqueAskPrices.erase(uniqueAskPricesIt);
        }
        else
        {
            uniqueAskPricesIt->second -= 1;
        }
                
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "Cancel ASK orderId [" << orderId << "] qty [" << qty << "] price [" << price << "]" << std::endl;
        
        Errors errors;
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto ret = FH.cancelSellOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        RC_ASSERT(true == ret);
        RC_ASSERT(FH.cancelBuyOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose) == false);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == 0U);
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == 0U);
        
        ++nbTests;
    };
    rc::check("Cancel Sell Orders", cancelSellOrders);
    rc::check("Cancel Sell Orders", cancelSellOrders);
    if (nbTests)
    {
        std::cout << "Cancel Sell Orders perfs : [" << time_span1/nbTests << "] (in ns)" << std::endl;
    }
#endif
    return 0;
}

