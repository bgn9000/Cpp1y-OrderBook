#include "FeedHandler.h"

#include "Parser.h"

bool FeedHandler::processMessage(const std::string& line)
{
//    std::cout << line << std::endl;  
    Parser p;
    if (!p.parse(line)) return false;
    switch(p.getAction())
    {
    case static_cast<char>(Parser::Action::ADD):
        if (unlikely(buyOrders_.find(p.getOrderId()) != buyOrders_.end() || 
                     sellOrders_.find(p.getOrderId()) != sellOrders_.end()))
        {
            std::cerr << "Duplicate orderId [" << p.getOrderId() << "], new order rejected" << std::endl;
            return false;
        }
        if (p.getSide() == static_cast<char>(Parser::Side::BUY))
        {
            return NewBuyOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()});
        } 
        else
        {
            return NewSellOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()});
        }
    case static_cast<char>(Parser::Action::CANCEL):
        if (p.getSide() == static_cast<char>(Parser::Side::BUY))
        {
            return CancelBuyOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()});
        } 
        else
        {
            return CancelSellOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()});
        }
    case static_cast<char>(Parser::Action::MODIFY):
        if (p.getSide() == static_cast<char>(Parser::Side::BUY))
        {
            return ModifyBuyOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()});
        } 
        else
        {
            return ModifySellOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()});
        }
    case static_cast<char>(Parser::Action::TRADE):
        break;
    }
    return false;
}

bool FeedHandler::NewBuyOrder(OrderId orderId, FeedHandler::Order&& order)
{
    auto itBids = std::lower_bound(bids_.begin(), bids_.end(), getPrice(order), 
        [](Order& o, Price p) -> bool
        {
            return (getPrice(o) < p);
        });
    if (itBids == bids_.end())
    {
        bids_.push_back(order);
    }
    else if (getPrice(*itBids) == getPrice(order))
    {
        getQty(*itBids) += getQty(order);
    }
    else
    {
        bids_.insert(itBids, order);
    }
    buyOrders_.emplace(orderId, std::move(order));
    return true;
}

bool FeedHandler::NewSellOrder(OrderId orderId, FeedHandler::Order&& order)
{    
    auto itAsks = std::lower_bound(asks_.begin(), asks_.end(), getPrice(order), 
        [](Order& o, Price p) -> bool
        {
            return (getPrice(o) < p);
        });
    if (itAsks == asks_.end())
    {
        asks_.push_back(order);
    }
    else if (getPrice(*itAsks) == getPrice(order))
    {
        getQty(*itAsks) += getQty(order);
    }
    else
    {
        asks_.insert(itAsks, order);
    }
    sellOrders_.emplace(orderId, std::move(order));
    return true;
}

bool FeedHandler::CancelBuyOrder(OrderId orderId, FeedHandler::Order&& order)
{
    auto itOrder = buyOrders_.find(orderId);
    if (unlikely(itOrder == buyOrders_.end()))
    {
        std::cerr << "Unknown buy orderId [" << orderId << "], cancel order impossible" << std::endl;
        return false;
    }
    if (unlikely(getQty(itOrder->second) != getQty(order) || getPrice(itOrder->second) != getPrice(order)))
    {
        std::cerr << "found buy orderId [" << orderId << "] but order info differs, cancel order rejected" << std::endl;
        return false;
    }
    
    auto ret = true;
    auto itBids = std::lower_bound(bids_.begin(), bids_.end(), getPrice(order), 
        [](Order& o, Price p) -> bool
        {
            return (getPrice(o) < p);
        });
    if (likely(itBids != bids_.end() && getPrice(*itBids) == getPrice(order)))
    {
        getQty(*itBids) -= getQty(order);
        if (getQty(*itBids) == 0)
        {
            bids_.erase(itBids);
        }
    }
    else
    {
        std::cerr << "Limit not found for orderId [" << orderId << "], cancel order failed" << std::endl;
        ret = false;
    }
    
    buyOrders_.erase(itOrder);
    return ret;
}

bool FeedHandler::CancelSellOrder(OrderId orderId, FeedHandler::Order&& order)
{
    auto itOrder = sellOrders_.find(orderId);
    if (unlikely(itOrder == sellOrders_.end()))
    {
        std::cerr << "Unknown sell orderId [" << orderId << "], cancel order impossible" << std::endl;
        return false;
    }
    if (unlikely(getQty(itOrder->second) != getQty(order) || getPrice(itOrder->second) != getPrice(order)))
    {
        std::cerr << "found sell orderId [" << orderId << "] but order info differs, cancel order rejected" << std::endl;
        return false;
    }
    
    auto ret = true;
    auto itAsks = std::lower_bound(asks_.begin(), asks_.end(), getPrice(order), 
        [](Order& o, Price p) -> bool
        {
            return (getPrice(o) < p);
        });
    if (likely(itAsks != asks_.end() && getPrice(*itAsks) == getPrice(order)))
    {
        getQty(*itAsks) -= getQty(order);
        if (getQty(*itAsks) == 0)
        {
            asks_.erase(itAsks);
        }
    }
    else
    {
        std::cerr << "Limit not found for orderId [" << orderId << "], cancel order failed" << std::endl;
        ret = false;
    }
    
    sellOrders_.erase(itOrder);
    return ret;
}

bool FeedHandler::ModifyBuyOrder(OrderId orderId, FeedHandler::Order&& order)
{
    auto itOrder = buyOrders_.find(orderId);
    if (unlikely(itOrder == buyOrders_.end()))
    {
        std::cerr << "Unknown buy orderId [" << orderId << "], modify order impossible" << std::endl;
        return false;
    }
    if (unlikely(getPrice(itOrder->second) != getPrice(order)))
    {
        std::cerr << "found buy orderId [" << orderId << "] but price differs, modify order rejected" << std::endl;
        return false;
    }

    auto ret = true;
    auto itBids = std::lower_bound(bids_.begin(), bids_.end(), getPrice(order), 
        [](Order& o, Price p) -> bool
        {
            return (getPrice(o) < p);
        });
    if (likely(itBids != bids_.end() && getPrice(*itBids) == getPrice(order)))
    {
        getQty(*itBids) += (getQty(order) - getQty(itOrder->second));
        if (getQty(*itBids) == 0)
        {
            bids_.erase(itBids);
        }
    }
    else
    {
        std::cerr << "Limit not found for orderId [" << orderId << "], modify order failed" << std::endl;
        ret = false;
    }

    itOrder->second = std::move(order);
    return true;
}

bool FeedHandler::ModifySellOrder(OrderId orderId, FeedHandler::Order&& order)
{
    auto itOrder = sellOrders_.find(orderId);
    if (unlikely(itOrder == sellOrders_.end()))
    {
        std::cerr << "Unknown sell orderId [" << orderId << "], modify order impossible" << std::endl;
        return false;
    }
    if (unlikely(getPrice(itOrder->second) != getPrice(order)))
    {
        std::cerr << "found sell orderId [" << orderId << "] but price differs, modify order rejected" << std::endl;
        return false;
    }

    auto ret = true;
    auto itAsks = std::lower_bound(asks_.begin(), asks_.end(), getPrice(order), 
        [](Order& o, Price p) -> bool
        {
            return (getPrice(o) < p);
        });
    if (likely(itAsks != asks_.end() && getPrice(*itAsks) == getPrice(order)))
    {
        getQty(*itAsks) += (getQty(order) - getQty(itOrder->second));
        if (getQty(*itAsks) == 0)
        {
            asks_.erase(itAsks);
        }
    }
    else
    {
        std::cerr << "Limit not found for orderId [" << orderId << "], modify order failed" << std::endl;
        ret = false;
    }

    itOrder->second = std::move(order);
    return true;
}

