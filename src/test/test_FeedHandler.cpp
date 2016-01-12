//#include <gtest/gtest.h>
#include <rapidcheck.h>

#include "FeedHandler.h"
#include "StrStream.h"

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <iterator>
#include <set>

#include <thread>
#include <future>
#include <condition_variable>
#include <mutex>

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
    
    inline void printCurrentOrderBook(const int verbose = 0) const
    {
        if (likely(0 == verbose))
        {
            std::ofstream null("/dev/null");
            FeedHandler::printCurrentOrderBook(static_cast<std::ostream&>(null));        
        }
        else FeedHandler::printCurrentOrderBook(std::cout);
    }
    
    bool checkBids(int line)
    {
        const auto len = bids_.size();
        if (0U == len) return true;
        Price prevPrice = getPrice(bids_[0]);
        for (auto i = 1U; i < len; ++i)
        {
            Price newPrice = getPrice(bids_[i]);
            if (newPrice > prevPrice) return false;
            prevPrice = newPrice;
        }
        return true;
    }
    bool checkAsks(int line)
    {
        const auto len = asks_.size();
        if (0U == len) return true;
        Price prevPrice = getPrice(asks_[0]);
        for (auto i = 1U; i < len; ++i)
        {
            Price newPrice = getPrice(asks_[i]);
            if (newPrice < prevPrice) return false;
            prevPrice = newPrice;
        }
        return true;
    }
    
    inline std::deque<Limit> copyBids() { return bids_; }
    inline std::deque<Limit> copyAsks() { return asks_; }
    
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
    
    std::cout << "Alignment for AggregatedQty = " << alignof(AggregatedQty) << std::endl;
    std::cout << "Alignment for Price = "  << alignof(Price) << std::endl;
    std::cout << "Alignment for Limit = "  << alignof(FeedHandler::Limit) << " whereas size is " << sizeof(FeedHandler::Limit) << std::endl;
    
    
    auto time_span1 = 0ULL, time_span2 = 0ULL;
    auto nbTests = 0U;
    rcFeedHandler FH, FH_prefilled;
    
#if 0
    std::deque<double> bids;
    auto insertBids = [&](double price)
    {
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto itBids = std::lower_bound(bids.begin(), bids.end(), price, std::greater<double>());
        bids.insert(itBids, price);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
//        std::cout << " " << duration_cast<nanoseconds>(end - start).count();
    };

    for (int i = 300000; i >= 100; i -= 2) bids.push_back(static_cast<double>(std::rand())/RAND_MAX+i);
    auto size1 = bids.size();
    for (int i = 299999 ; i > 0; i -= 200) insertBids(static_cast<double>(std::rand())/RAND_MAX+i);
    nbTests = bids.size() - size1;
    std::cout << "\nPerfs : [" << time_span1/nbTests << "] (in ns)" << std::endl;
//    std::copy(bids.begin(), bids.end(), std::ostream_iterator<double>(std::cout, " "));
//    std::cout << std::endl;
#endif
#if 0
    time_span1 = time_span2 = 0ULL;
    std::deque<double> asks;
    auto insertAsks = [&](double price)
    {
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto itAsks = std::lower_bound(asks.begin(), asks.end(), price);
        asks.insert(itAsks, price);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
//        std::cout << " " << duration_cast<nanoseconds>(end - start).count();
    };

    for (int i = 100; i <= 300000; i += 2) asks.push_back(static_cast<double>(std::rand())/RAND_MAX+i);
    size1 = asks.size();
    for (int i = 1; i <= 299999; i += 200) insertAsks(static_cast<double>(std::rand())/RAND_MAX+i);
    nbTests = asks.size() - size1;
    std::cout << "\nPerfs : [" << time_span1/nbTests << "] (in ns)" << std::endl;
//    std::copy(asks.begin(), asks.end(), std::ostream_iterator<double>(std::cout, " "));
//    std::cout << std::endl;
#endif
#if 1
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    {    
        OrderId buyOrderId = maxOrderId/2;
        Price buyPrice = maxOrderPrice;
        OrderId sellOrderId = buyOrderId+1;
        Price sellPrice = 0.0;
        auto prefill = [&]()
        {
            auto ret = false;
            auto stepPrice = static_cast<Price>(*rc::gen::inRange(1, 9'000'000)) / 1000.0;
            for (int cpt = 0; cpt < 1000; ++cpt)
            {
                buyPrice -= stepPrice;
                FH_prefilled.uniqueBidPrices[buyPrice] = 1;
                sellPrice += stepPrice;
                FH_prefilled.uniqueAskPrices[sellPrice] = 1;
                const Quantity qty = *rc::gen::inRange<Quantity>(10, maxOrderQty);
                FH_prefilled.buyOrders[buyOrderId] = FeedHandler::Order{qty, buyPrice};
                FH_prefilled.sellOrders[sellOrderId] = FeedHandler::Order{qty, sellPrice};
                
                Errors errors;
                high_resolution_clock::time_point start = high_resolution_clock::now();
                ret = FH_prefilled.newBuyOrder(buyOrderId, FeedHandler::Order{qty, buyPrice}, errors, verbose);
                high_resolution_clock::time_point end = high_resolution_clock::now();
                time_span1 += duration_cast<nanoseconds>(end - start).count();
                
                start = high_resolution_clock::now();
                ret &= FH_prefilled.newSellOrder(sellOrderId, FeedHandler::Order{qty, sellPrice}, errors, verbose);
                end = high_resolution_clock::now();
                time_span2 += duration_cast<nanoseconds>(end - start).count();
                
                buyOrderId += 2;
                sellOrderId += 2;
                
                ++nbTests;
            }
            RC_ASSERT(true == ret);
            RC_ASSERT(FH_prefilled.getNbBuyOrders() == FH_prefilled.buyOrders.size());
            RC_ASSERT(FH_prefilled.getNbSellOrders() == FH_prefilled.sellOrders.size());
            RC_ASSERT(FH_prefilled.getNbBids() == FH_prefilled.uniqueBidPrices.size());
            RC_ASSERT(FH_prefilled.getNbAsks() == FH_prefilled.uniqueAskPrices.size());
        };

        rc::check("Prefill New Buy/Sell Orders", prefill);
        if (nbTests)
        {
            std::cout << "Prefill New Buy Orders (" << FH_prefilled.getNbBuyOrders() << ") perfs : [" << time_span1/nbTests 
                << "] and New Sell Orders (" << FH_prefilled.getNbSellOrders() << ") perfs : [" << time_span2/nbTests 
                << "] (in ns)" << std::endl;
        }
        std::cout << "Last buyPrice=" << buyPrice << " sellPrice=" << sellPrice << std::endl;
    }
    FH_prefilled.checkBids(__LINE__);
    FH_prefilled.checkAsks(__LINE__);
#endif
#if 1
    {
        auto nbDepths = std::max(FH_prefilled.getNbBids(), FH_prefilled.getNbAsks());
        
        high_resolution_clock::time_point start = high_resolution_clock::now();
        FH_prefilled.printCurrentOrderBook();
        high_resolution_clock::time_point end = high_resolution_clock::now();
        std::cout << "\nPrint prefilled (" << nbDepths << ") orderbook perfs\t\t\t: [" 
            << duration_cast<nanoseconds>(end - start).count() << "] (in ns)" << std::endl;
                
        start = high_resolution_clock::now();
        std::deque<FeedHandler::Limit> bids_copy = FH_prefilled.copyBids();
        std::deque<FeedHandler::Limit> asks_copy = FH_prefilled.copyAsks();
        end = high_resolution_clock::now();
        std::cout << "Copy prefilled (" << nbDepths << ") orderbook perfs\t\t\t\t: [" 
            << duration_cast<nanoseconds>(end - start).count() << "] (in ns)" << std::endl;
            
        FeedHandler::Limit fakeLimit{100, 1.0};
        start = high_resolution_clock::now();
        bids_copy.insert(bids_copy.begin(), fakeLimit);
        asks_copy.insert(asks_copy.begin(), fakeLimit);
        end = high_resolution_clock::now();
        std::cout << "Insert at beginning of prefilled (" << nbDepths << ") orderbook perfs\t: [" 
            << duration_cast<nanoseconds>(end - start).count() << "] (in ns)" << std::endl;
        
        auto async_printCurrentOrderBook = [&]() { FH_prefilled.printCurrentOrderBook(); };
        start = high_resolution_clock::now();
        auto fut = std::async(std::launch::async, async_printCurrentOrderBook);
        end = high_resolution_clock::now();
        std::cout << "Async print prefilled (" << nbDepths << ") orderbook perfs\t\t\t: [" 
            << duration_cast<nanoseconds>(end - start).count() << "] (in ns)" << std::endl;
        fut.get();
        
        std::mutex mut;
        std::condition_variable cond;
        auto threaded_printCurrentOrderBook = [&]() 
        {
            {
                std::unique_lock<std::mutex> guard(mut);
                cond.notify_one();
            }
            {
                std::unique_lock<std::mutex> guard(mut);
                cond.wait(guard);
                FH_prefilled.printCurrentOrderBook(); 
            }
        };
        std::thread thr(threaded_printCurrentOrderBook);
        {
            std::unique_lock<std::mutex> guard(mut);
            cond.wait(guard);
        }
        start = high_resolution_clock::now();
        {
            std::unique_lock<std::mutex> guard(mut);
            cond.notify_one();
        }
        end = high_resolution_clock::now();
        std::cout << "Threaded print prefilled (" << nbDepths << ") orderbook perfs\t\t: [" 
            << duration_cast<nanoseconds>(end - start).count() << "] (in ns)" << std::endl;
        thr.join();
    }
#endif
#if 1
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("New Buy Orders", [&]()
    {
        auto price = static_cast<Price>(*rc::gen::inRange(1, 99)) / *rc::gen::inRange(100, 10'000) 
            + *rc::gen::inRange(1, maxOrderPrice/2);
        while (FH.uniqueBidPrices.find(price) != FH.uniqueBidPrices.end())
        { price += 1.0 / *rc::gen::inRange(2, 100'000); }
        FH.uniqueBidPrices[price] = 1;
        const Quantity qty = *rc::gen::inRange(10, maxOrderQty);
        auto orderId = *rc::gen::suchThat(rc::gen::inRange(1, 1000), [](int x) { return (x % 2) == 0; });
        while (FH.buyOrders.find(orderId) != FH.buyOrders.end())
        { 
            orderId = *rc::gen::suchThat(rc::gen::inRange(1, 1000), [](int x) { return (x % 2) == 0; });
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
        RC_ASSERT(1ULL == errors.duplicateOrderIds);
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbSellOrders() == 0U);
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        RC_ASSERT(FH.getNbAsks() == 0U);
        
        while (FH_prefilled.uniqueBidPrices.find(price) != FH_prefilled.uniqueBidPrices.end())
        { price += 0.001; }
        FH_prefilled.uniqueBidPrices[price] = 1;
        FH_prefilled.buyOrders[orderId] = FeedHandler::Order{qty, price};

        start = high_resolution_clock::now();
        ret = FH_prefilled.newBuyOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(true == ret);
        RC_ASSERT(FH_prefilled.getNbBuyOrders() == FH_prefilled.buyOrders.size());
        RC_ASSERT(FH_prefilled.getNbBids() == FH_prefilled.uniqueBidPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "New Buy Orders (" << FH.getNbBuyOrders() << ") perfs : [" << time_span1/nbTests 
            << "] and in prefilled (" << FH_prefilled.getNbBuyOrders() << ") perfs : [" << time_span2/nbTests
            << "] (in ns)" << std::endl;
    }
    FH_prefilled.checkBids(__LINE__);
    if (FH.checkBids(__LINE__) == false)
    {
        std::cerr << "Check bids failed!" << std::endl;
        FH.printCurrentOrderBook(verbose);
        return -1;
    }
    
    {
        high_resolution_clock::time_point start = high_resolution_clock::now();
        FH.printCurrentOrderBook(verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        std::cout << "Print half filled orderbook (" << FH.getNbBids() << ") perfs : [" 
            << duration_cast<nanoseconds>(end - start).count() << "] (in ns)" << std::endl;
    }

    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("New Sell Orders", [&]()
    {
        auto price = static_cast<Price>(*rc::gen::inRange(1, 99)) / *rc::gen::inRange(100, 10'000) 
            + *rc::gen::inRange(maxOrderPrice/2, maxOrderPrice);
        while (FH.uniqueAskPrices.find(price) != FH.uniqueAskPrices.end())
        { price += 1.0 / *rc::gen::inRange(2, 100'000); }
        FH.uniqueAskPrices[price] = 1;
        const Quantity qty = *rc::gen::inRange(10, maxOrderQty);
        auto orderId = *rc::gen::suchThat(rc::gen::inRange(1, 1000), [](int x) { return (x % 2) != 0; });
        while (FH.sellOrders.find(orderId) != FH.sellOrders.end())
        { 
            orderId = *rc::gen::suchThat(rc::gen::inRange(1, 1000), [](int x) { return (x % 2) != 0; });
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
        RC_ASSERT(FH.newBuyOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose) == false);
        RC_ASSERT(1ULL == errors.duplicateOrderIds);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        while (FH_prefilled.uniqueAskPrices.find(price) != FH_prefilled.uniqueAskPrices.end())
        { price += 0.001; }
        FH_prefilled.uniqueAskPrices[price] = 1;
        FH_prefilled.sellOrders[orderId] = FeedHandler::Order{qty, price};
        
        start = high_resolution_clock::now();
        ret = FH_prefilled.newSellOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(true == ret);
        RC_ASSERT(FH_prefilled.getNbBuyOrders() == FH_prefilled.buyOrders.size());
        RC_ASSERT(FH_prefilled.getNbBids() == FH_prefilled.uniqueBidPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "New Sell Orders (" << FH.getNbSellOrders() << ") perfs : [" << time_span1/nbTests
            << "] and in prefilled (" << FH_prefilled.getNbSellOrders() << ") perfs : [" << time_span2/nbTests
            << "] (in ns)" << std::endl;
    }
    FH_prefilled.checkAsks(__LINE__);
    if (FH.checkAsks(__LINE__) == false)
    {
        std::cerr << "Check asks failed!" << std::endl;
        FH.printCurrentOrderBook(verbose);
        return -1;
    }
    
    {
        auto nbDepths = std::max(FH.getNbBids(), FH.getNbAsks());
        high_resolution_clock::time_point start = high_resolution_clock::now();
        FH.printCurrentOrderBook(verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        std::cout << "Print (" << nbDepths << ") orderbook perfs : [" 
            << duration_cast<nanoseconds>(end - start).count() << "] (in ns)" << std::endl;
    }
#endif
#if 1
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    std::map<OrderId, FeedHandler::Order> buyOrders_copie = FH.buyOrders;
    auto buyOrderIt = buyOrders_copie.begin();
    std::map<OrderId, FeedHandler::Order> buyPrefilledOrders_copie = FH_prefilled.buyOrders;
    auto buyPrefilledOrderIt = buyPrefilledOrders_copie.begin();
    rc::check("Duplicate New Buy Orders", [&]()
    {
        auto sameOrderId = buyOrderIt->first;
        auto sameOrder = buyOrderIt->second;
        ++buyOrderIt;
        
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "Duplicate New BUY orderId [" << sameOrderId << "] qty [" << FeedHandler::getQty(sameOrder) 
            << "] same price [" << FeedHandler::getPrice(sameOrder) << "]" << std::endl;
        
        Errors errors;
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto ret = FH.newBuyOrder(sameOrderId, std::forward<FeedHandler::Order>(sameOrder), errors, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.duplicateOrderIds);
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        sameOrderId = buyPrefilledOrderIt->first;
        sameOrder = buyPrefilledOrderIt->second;
        ++buyPrefilledOrderIt;
        
        memset((void*)&errors, 0, sizeof(Errors));
        start = high_resolution_clock::now();
        ret = FH_prefilled.newBuyOrder(sameOrderId, std::forward<FeedHandler::Order>(sameOrder), errors, verbose);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.duplicateOrderIds);
        RC_ASSERT(FH_prefilled.getNbBuyOrders() == FH_prefilled.buyOrders.size());
        RC_ASSERT(FH_prefilled.getNbBids() == FH_prefilled.uniqueBidPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Duplicate New Buy Orders (" << FH.getNbBuyOrders() << ") perfs : [" << time_span1/nbTests 
            << "] and in prefilled (" << FH_prefilled.getNbBuyOrders() << ") : [" << time_span2/nbTests
            << "] (in ns)" << std::endl;
    }

    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    std::map<OrderId, FeedHandler::Order> sellOrders_copie = FH.sellOrders;
    auto sellOrderIt = sellOrders_copie.begin();
    std::map<OrderId, FeedHandler::Order> sellPrefilledOrders_copie = FH_prefilled.sellOrders;
    auto sellPrefilledOrderIt = sellPrefilledOrders_copie.begin();
    rc::check("Duplicate New Sell Orders", [&]()
    {
        auto sameOrderId = sellOrderIt->first;
        auto sameOrder = sellOrderIt->second;
        ++sellOrderIt;
        
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "Duplicate New SELL orderId [" << sameOrderId << "] qty [" << FeedHandler::getQty(sameOrder) 
            << "] same price [" << FeedHandler::getPrice(sameOrder) << "]" << std::endl;
        
        Errors errors;
        high_resolution_clock::time_point start = high_resolution_clock::now();
        auto ret = FH.newSellOrder(sameOrderId, std::forward<FeedHandler::Order>(sameOrder), errors, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.duplicateOrderIds);
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());

        sameOrderId = sellPrefilledOrderIt->first;
        sameOrder = sellPrefilledOrderIt->second;
        ++sellPrefilledOrderIt;
        
        memset((void*)&errors, 0, sizeof(Errors));
        start = high_resolution_clock::now();
        ret = FH_prefilled.newSellOrder(sameOrderId, std::forward<FeedHandler::Order>(sameOrder), errors, verbose);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.duplicateOrderIds);
        RC_ASSERT(FH_prefilled.getNbSellOrders() == FH_prefilled.sellOrders.size());
        RC_ASSERT(FH_prefilled.getNbAsks() == FH_prefilled.uniqueAskPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Duplicate New Sell Orders (" << FH.getNbSellOrders() << ") perfs : [" << time_span1/nbTests 
            << "] and in prefilled (" << FH_prefilled.getNbSellOrders() << ") : [" << time_span2/nbTests
            << "] (in ns)" << std::endl;
    }
    
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    buyOrderIt = buyOrders_copie.begin();
    buyPrefilledOrderIt = buyPrefilledOrders_copie.begin();
    rc::check("Add New Buy Orders with same price", [&]()
    {
        auto price = FeedHandler::getPrice(buyOrderIt->second);
        FH.uniqueBidPrices[price] = 2;
        ++buyOrderIt;
        const Quantity qty = *rc::gen::inRange(10, maxOrderQty);
        auto orderId = *rc::gen::suchThat(rc::gen::inRange(1001, 2000), [](int x) { return (x % 2) == 0; });
        while (FH.buyOrders.find(orderId) != FH.buyOrders.end())
        { 
            orderId = *rc::gen::suchThat(rc::gen::inRange(1001, 2000), [](int x) { return (x % 2) == 0; });
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
        RC_ASSERT(1ULL == errors.duplicateOrderIds);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        price = FeedHandler::getPrice(buyPrefilledOrderIt->second);
        FH_prefilled.uniqueBidPrices[price] = 2;
        FH_prefilled.buyOrders[orderId] = FeedHandler::Order{qty, price};
        ++buyPrefilledOrderIt;
        
        start = high_resolution_clock::now();
        ret = FH_prefilled.newBuyOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(true == ret);
        RC_ASSERT(FH_prefilled.getNbBuyOrders() == FH_prefilled.buyOrders.size());
        RC_ASSERT(FH_prefilled.getNbBids() == FH_prefilled.uniqueBidPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Add New Buy Orders with same price (" << FH.getNbBuyOrders() << ") perfs : [" << time_span1/nbTests 
            << "] and in prefilled (" << FH_prefilled.getNbBuyOrders() << ") : [" << time_span2/nbTests
            << "] (in ns)" << std::endl;
    }
    FH_prefilled.checkBids(__LINE__);
    if (FH.checkBids(__LINE__) == false)
    {
        std::cerr << "Check bids failed!" << std::endl;
        FH.printCurrentOrderBook(verbose);
        return -1;
    }
    {
        auto nbDepths = std::max(FH.getNbBids(), FH.getNbAsks());
        high_resolution_clock::time_point start = high_resolution_clock::now();
        FH.printCurrentOrderBook(verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        std::cout << "Print (" << nbDepths << ") orderbook perfs : [" 
            << duration_cast<nanoseconds>(end - start).count() << "] (in ns)" << std::endl;
    }
    
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    sellOrderIt = sellOrders_copie.begin();
    sellPrefilledOrderIt = sellPrefilledOrders_copie.begin();
    rc::check("Add New Sell Orders with same price", [&]()
    {
        auto price = FeedHandler::getPrice(sellOrderIt->second);
        FH.uniqueAskPrices[price] = 2;
        ++sellOrderIt;
        const Quantity qty = *rc::gen::inRange(10, maxOrderQty);
        auto orderId = *rc::gen::suchThat(rc::gen::inRange(1001, 2000), [](int x) { return (x % 2) != 0; });
        while (FH.sellOrders.find(orderId) != FH.sellOrders.end())
        {
            orderId = *rc::gen::suchThat(rc::gen::inRange(1001, 2000), [](int x) { return (x % 2) != 0; });
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
        RC_ASSERT(1ULL == errors.duplicateOrderIds);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        price = FeedHandler::getPrice(sellPrefilledOrderIt->second);
        FH_prefilled.uniqueAskPrices[price] += 1;
        FH_prefilled.sellOrders[orderId] = FeedHandler::Order{qty, price};
        ++sellPrefilledOrderIt;
        
        start = high_resolution_clock::now();
        ret = FH_prefilled.newSellOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(true == ret);
        RC_ASSERT(FH_prefilled.getNbSellOrders() == FH_prefilled.sellOrders.size());
        RC_ASSERT(FH_prefilled.getNbAsks() == FH_prefilled.uniqueAskPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Add New Sell Orders with same price (" << FH.getNbSellOrders() << ") perfs : [" << time_span1/nbTests 
            << "] and in prefilled (" << FH_prefilled.getNbSellOrders() << ") : [" << time_span2/nbTests
            << "] (in ns)" << std::endl;
    }
    FH_prefilled.checkAsks(__LINE__);
    if (FH.checkAsks(__LINE__) == false)
    {
        std::cerr << "Check asks failed!" << std::endl;
        FH.printCurrentOrderBook(verbose);
        return -1;
    }
    {
        auto nbDepths = std::max(FH.getNbBids(), FH.getNbAsks());
        high_resolution_clock::time_point start = high_resolution_clock::now();
        FH.printCurrentOrderBook(verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        std::cout << "Print (" << nbDepths << ") orderbook perfs : [" 
            << duration_cast<nanoseconds>(end - start).count() << "] (in ns)" << std::endl;
    }
#endif
#if 1
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    buyOrderIt = FH.buyOrders.begin();
    buyPrefilledOrderIt = FH_prefilled.buyOrders.begin();
    rc::check("Modify Buy Orders", [&]()
    {
        auto orderId = buyOrderIt->first;
        const Quantity newqty = *rc::gen::inRange(10, maxOrderQty);
        FeedHandler::getQty(buyOrderIt->second) = newqty;
        auto price = FeedHandler::getPrice(buyOrderIt->second);
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
        RC_ASSERT(1ULL == errors.modifiesWithUnknownOrderId);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        memset((void*)&errors, 0, sizeof(Errors));
        ret = FH.modifyBuyOrder(orderId+2000, FeedHandler::Order{newqty, price}, errors, verbose);
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.modifiesWithUnknownOrderId);
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        memset((void*)&errors, 0, sizeof(Errors));
        ret = FH.modifyBuyOrder(orderId, FeedHandler::Order{newqty, price+0.1}, errors, verbose);
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.modifiesNotMatchedPrice);
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        orderId = buyPrefilledOrderIt->first;
        FeedHandler::getQty(buyPrefilledOrderIt->second) = newqty;
        price = FeedHandler::getPrice(buyPrefilledOrderIt->second);
        ++buyPrefilledOrderIt;
        
        memset((void*)&errors, 0, sizeof(Errors));
        start = high_resolution_clock::now();
        ret = FH_prefilled.modifyBuyOrder(orderId, FeedHandler::Order{newqty, price}, errors, verbose);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(true == ret);
        RC_ASSERT(FH_prefilled.getNbBuyOrders() == FH_prefilled.buyOrders.size());
        RC_ASSERT(FH_prefilled.getNbBids() == FH_prefilled.uniqueBidPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Modify Buy Orders (" << FH.getNbBuyOrders() << ") perfs : [" << time_span1/nbTests 
            << "] and in prefilled (" << FH_prefilled.getNbBuyOrders() << ") : [" << time_span2/nbTests
            << "] (in ns)" << std::endl;
    }
    FH_prefilled.checkBids(__LINE__);
    if (FH.checkBids(__LINE__) == false)
    {
        std::cerr << "Check bids failed!" << std::endl;
        FH.printCurrentOrderBook(verbose);
        return -1;
    }
    {
        auto nbDepths = std::max(FH.getNbBids(), FH.getNbAsks());
        high_resolution_clock::time_point start = high_resolution_clock::now();
        FH.printCurrentOrderBook(verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        std::cout << "Print (" << nbDepths << ") orderbook perfs : [" 
            << duration_cast<nanoseconds>(end - start).count() << "] (in ns)" << std::endl;
    }
    
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    sellOrderIt = FH.sellOrders.begin();
    sellPrefilledOrderIt = FH_prefilled.sellOrders.begin();
    rc::check("Modify Sell Orders", [&]()
    {
        auto orderId = sellOrderIt->first;
        const Quantity newqty = *rc::gen::inRange(10, maxOrderQty);
        FeedHandler::getQty(sellOrderIt->second) = newqty;
        auto price = FeedHandler::getPrice(sellOrderIt->second);
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
        RC_ASSERT(1ULL == errors.modifiesWithUnknownOrderId);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        memset((void*)&errors, 0, sizeof(Errors));
        ret = FH.modifySellOrder(orderId+2000, FeedHandler::Order{newqty, price}, errors, verbose);
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.modifiesWithUnknownOrderId);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        
        memset((void*)&errors, 0, sizeof(Errors));
        ret = FH.modifySellOrder(orderId, FeedHandler::Order{newqty, price+0.1}, errors, verbose);
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.modifiesNotMatchedPrice);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        
        orderId = sellPrefilledOrderIt->first;
        FeedHandler::getQty(sellPrefilledOrderIt->second) = newqty;
        price = FeedHandler::getPrice(sellPrefilledOrderIt->second);
        ++sellPrefilledOrderIt;
        
        memset((void*)&errors, 0, sizeof(Errors));
        start = high_resolution_clock::now();
        ret = FH_prefilled.modifySellOrder(orderId, FeedHandler::Order{newqty, price}, errors, verbose);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(true == ret);
        RC_ASSERT(FH_prefilled.getNbSellOrders() == FH_prefilled.sellOrders.size());
        RC_ASSERT(FH_prefilled.getNbAsks() == FH_prefilled.uniqueAskPrices.size());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Modify Sell Orders (" << FH.getNbSellOrders() << ") perfs : [" << time_span1/nbTests 
            << "] and in prefilled (" << FH_prefilled.getNbSellOrders() << ") : [" << time_span2/nbTests
            << "] (in ns)" << std::endl;
    }
    FH_prefilled.checkAsks(__LINE__);
    if (FH.checkAsks(__LINE__) == false)
    {
        std::cerr << "Check asks failed!" << std::endl;
        FH.printCurrentOrderBook(verbose);
        return -1;
    }
    {
        auto nbDepths = std::max(FH.getNbBids(), FH.getNbAsks());
        high_resolution_clock::time_point start = high_resolution_clock::now();
        FH.printCurrentOrderBook(verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        std::cout << "Print (" << nbDepths << ") orderbook perfs : [" 
            << duration_cast<nanoseconds>(end - start).count() << "] (in ns)" << std::endl;
    }
#endif
#if 1
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    auto cancelBuyOrders = [&]()
    {
        buyOrderIt = FH.buyOrders.begin();
        auto orderId = buyOrderIt->first;
        auto qty = FeedHandler::getQty(buyOrderIt->second);
        auto price = FeedHandler::getPrice(buyOrderIt->second);
        FH.buyOrders.erase(buyOrderIt);
        auto uniqueBidPricesIt = FH.uniqueBidPrices.find(price);
        RC_ASSERT(uniqueBidPricesIt != FH.uniqueBidPrices.end());
        if (1 == uniqueBidPricesIt->second)
        {
            FH.uniqueBidPrices.erase(uniqueBidPricesIt);
        }
        else
        {
            uniqueBidPricesIt->second = 1;
        }
        
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "Cancel BUY orderId [" << orderId << "] qty [" << qty << "] price [" << price << "]" << std::endl;
        
        Errors errors;
        auto ret = FH.cancelBuyOrder(orderId+2000, FeedHandler::Order{qty, price}, errors, verbose);
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.cancelsWithUnknownOrderId);
        
        memset((void*)&errors, 0, sizeof(Errors));
        ret = FH.cancelBuyOrder(orderId, FeedHandler::Order{qty+1, price}, errors, verbose);
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.cancelsNotMatchedQtyOrPrice);
        
        memset((void*)&errors, 0, sizeof(Errors));
        ret = FH.cancelBuyOrder(orderId, FeedHandler::Order{qty, price+0.1}, errors, verbose);
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.cancelsNotMatchedQtyOrPrice);
        
        memset((void*)&errors, 0, sizeof(Errors));
        high_resolution_clock::time_point start = high_resolution_clock::now();
        ret = FH.cancelBuyOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(true == ret);
        RC_ASSERT(FH.cancelSellOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose) == false);
        RC_ASSERT(1ULL == errors.cancelsWithUnknownOrderId);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == FH.buyOrders.size());
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == FH.uniqueBidPrices.size());
        
        buyOrderIt = FH_prefilled.buyOrders.begin();
        orderId = buyOrderIt->first;
        qty = FeedHandler::getQty(buyOrderIt->second);
        price = FeedHandler::getPrice(buyOrderIt->second);
        FH_prefilled.buyOrders.erase(buyOrderIt);
        uniqueBidPricesIt = FH_prefilled.uniqueBidPrices.find(price);
        RC_ASSERT(uniqueBidPricesIt != FH_prefilled.uniqueBidPrices.end());
        if (1 == uniqueBidPricesIt->second)
        {
            FH_prefilled.uniqueBidPrices.erase(uniqueBidPricesIt);
        }
        else
        {
            uniqueBidPricesIt->second = 1;
        }
        
        memset((void*)&errors, 0, sizeof(Errors));
        start = high_resolution_clock::now();
        ret = FH_prefilled.cancelBuyOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(true == ret);
        RC_ASSERT(FH_prefilled.getNbBuyOrders() == FH_prefilled.buyOrders.size());
        RC_ASSERT(FH_prefilled.getNbBids() == FH_prefilled.uniqueBidPrices.size());
        
        ++nbTests;
    };
    rc::check("Cancel Buy Orders", cancelBuyOrders);
    rc::check("Cancel Buy Orders", cancelBuyOrders);
    if (nbTests)
    {
        std::cout << "Cancel Buy Orders (" << FH.getNbBuyOrders() << ") perfs : [" << time_span1/nbTests 
            << "] and in prefilled (" << FH_prefilled.getNbBuyOrders() << ") : [" << time_span2/nbTests
            << "] (in ns)" << std::endl;
    }
    
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    auto cancelSellOrders = [&]()
    {
        sellOrderIt = FH.sellOrders.begin();
        auto orderId = sellOrderIt->first;
        auto qty = FeedHandler::getQty(sellOrderIt->second);
        auto price = FeedHandler::getPrice(sellOrderIt->second);
        FH.sellOrders.erase(sellOrderIt);
        auto uniqueAskPricesIt = FH.uniqueAskPrices.find(price);
        RC_ASSERT(uniqueAskPricesIt != FH.uniqueAskPrices.end());
        if (1 == uniqueAskPricesIt->second)
        {
            FH.uniqueAskPrices.erase(uniqueAskPricesIt);
        }
        else
        {
            uniqueAskPricesIt->second = 1;
        }
                
        RC_LOG() /*std::cout*/ << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "Cancel ASK orderId [" << orderId << "] qty [" << qty << "] price [" << price << "]" << std::endl;
        
        Errors errors;
        auto ret = FH.cancelSellOrder(orderId+2000, FeedHandler::Order{qty, price}, errors, verbose);
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.cancelsWithUnknownOrderId);
        
        memset((void*)&errors, 0, sizeof(Errors));
        ret = FH.cancelSellOrder(orderId, FeedHandler::Order{qty+1, price}, errors, verbose);
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.cancelsNotMatchedQtyOrPrice);
        
        memset((void*)&errors, 0, sizeof(Errors));
        ret = FH.cancelSellOrder(orderId, FeedHandler::Order{qty, price+0.1}, errors, verbose);
        RC_ASSERT(false == ret);
        RC_ASSERT(1ULL == errors.cancelsNotMatchedQtyOrPrice);
        
        memset((void*)&errors, 0, sizeof(Errors));
        high_resolution_clock::time_point start = high_resolution_clock::now();
        ret = FH.cancelSellOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(true == ret);
        RC_ASSERT(FH.cancelBuyOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose) == false);
        RC_ASSERT(1ULL == errors.cancelsWithUnknownOrderId);
        RC_ASSERT(FH.getNbSellOrders() == FH.sellOrders.size());
        RC_ASSERT(FH.getNbBuyOrders() == 0U);
        RC_ASSERT(FH.getNbAsks() == FH.uniqueAskPrices.size());
        RC_ASSERT(FH.getNbBids() == 0U);
        
        ++nbTests;
        
        sellOrderIt = FH_prefilled.sellOrders.begin();
        orderId = sellOrderIt->first;
        qty = FeedHandler::getQty(sellOrderIt->second);
        price = FeedHandler::getPrice(sellOrderIt->second);
        FH_prefilled.sellOrders.erase(sellOrderIt);
        uniqueAskPricesIt = FH_prefilled.uniqueAskPrices.find(price);
        RC_ASSERT(uniqueAskPricesIt != FH_prefilled.uniqueAskPrices.end());
        if (1 == uniqueAskPricesIt->second)
        {
            FH_prefilled.uniqueAskPrices.erase(uniqueAskPricesIt);
        }
        else
        {
            uniqueAskPricesIt->second = 1;
        }
        
        memset((void*)&errors, 0, sizeof(Errors));
        start = high_resolution_clock::now();
        ret = FH_prefilled.cancelSellOrder(orderId, FeedHandler::Order{qty, price}, errors, verbose);
        end = high_resolution_clock::now();
        time_span2 += duration_cast<nanoseconds>(end - start).count();
        RC_ASSERT(true == ret);
        RC_ASSERT(FH_prefilled.getNbSellOrders() == FH_prefilled.sellOrders.size());
        RC_ASSERT(FH_prefilled.getNbAsks() == FH_prefilled.uniqueAskPrices.size());
    };
    rc::check("Cancel Sell Orders", cancelSellOrders);
    rc::check("Cancel Sell Orders", cancelSellOrders);
    if (nbTests)
    {
        std::cout << "Cancel Sell Orders (" << FH.getNbSellOrders() << ") perfs : [" << time_span1/nbTests 
            << "] and in prefilled (" << FH_prefilled.getNbSellOrders() << ") : [" << time_span2/nbTests
            << "] (in ns)" << std::endl;
    }
#endif
    return 0;
}

