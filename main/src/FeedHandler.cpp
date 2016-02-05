#include "FeedHandler.h"

#include "Parser.h"
#include "StrStream.h"

void FeedHandler::processMessage(const char* data, size_t dataLen, Errors& errors, const int verbose)
{
    Parser p;
    if (likely(p.parse(data, dataLen, errors, verbose)))
    {
        switch(p.getAction())
        {
        case static_cast<char>(Parser::Action::ADD):
            switch(p.getSide())
            {
            case static_cast<char>(Parser::Side::BUY):
                newBuyOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
                break;
            case static_cast<char>(Parser::Side::SELL):
                newSellOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
                break;
            default:
                ++errors.wrongSides;
                break;
            }
            break;
        case static_cast<char>(Parser::Action::CANCEL):
            switch(p.getSide())
            {
            case static_cast<char>(Parser::Side::BUY):
                cancelBuyOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
                break;
            case static_cast<char>(Parser::Side::SELL):
                cancelSellOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
                break;
            default:
                ++errors.wrongSides;
                break;
            }
            break;
        case static_cast<char>(Parser::Action::MODIFY):
            switch(p.getSide())
            {
            case static_cast<char>(Parser::Side::BUY):
                modifyBuyOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
                break;
            case static_cast<char>(Parser::Side::SELL):
                modifySellOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
                break;
            default:
                ++errors.wrongSides;
                break;
            }
            break;
        case static_cast<char>(Parser::Action::TRADE):
            queue_.push_back(Data('T', 0, 0, Trade{p.getQty(), p.getPrice()}));
            break;
        default: 
            ++errors.wrongActions;
            break;
        }
    }
}

void FeedHandler::newBuyOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose)
{
    if (unlikely(buyOrders_.find(orderId) != buyOrders_.end() || 
                 sellOrders_.find(orderId) != sellOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Duplicate buy orderId [" << orderId << "], new order rejected" << std::endl;
        ++errors.duplicateOrderIds;
        return;
    }
    auto itBids = std::lower_bound(bids_.begin(), bids_.end(), getPrice(order), 
        [](Limit& l, Price p) -> bool
        {
            return (getPrice(l) > p);
        });
    if (itBids == bids_.end() || getPrice(*itBids) != getPrice(order))
    {
        queue_.push_back(
            Data(static_cast<char>(Parser::Action::ADD), static_cast<char>(Parser::Side::BUY), 
                 static_cast<unsigned int>(itBids-bids_.begin()), order)
        );
        bids_.insert(itBids, order);
    }
    else
    {
        getQty(*itBids) += getQty(order);
        queue_.push_back(
            Data(static_cast<char>(Parser::Action::MODIFY), static_cast<char>(Parser::Side::BUY), 
                 static_cast<unsigned int>(itBids-bids_.begin()), *itBids)
        );
    }
    buyOrders_.emplace(orderId, std::forward<Order>(order));
}

void FeedHandler::newSellOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose)
{
    if (unlikely(buyOrders_.find(orderId) != buyOrders_.end() || 
                 sellOrders_.find(orderId) != sellOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Duplicate sell orderId [" << orderId << "], new order rejected" << std::endl;
        ++errors.duplicateOrderIds;
        return;
    }
    auto itAsks = std::lower_bound(asks_.begin(), asks_.end(), getPrice(order), 
        [](Limit& l, Price p) -> bool
        {
            return (getPrice(l) < p);
        });
    if (itAsks == asks_.end() || getPrice(*itAsks) != getPrice(order))
    {
        queue_.push_back(
            Data(static_cast<char>(Parser::Action::ADD), static_cast<char>(Parser::Side::SELL), 
                 static_cast<unsigned int>(itAsks-asks_.begin()), order)
        );
        asks_.insert(itAsks, order);
    }
    else    
    {
        getQty(*itAsks) += getQty(order);
        queue_.push_back(
            Data(static_cast<char>(Parser::Action::MODIFY), static_cast<char>(Parser::Side::SELL), 
                 static_cast<unsigned int>(itAsks-asks_.begin()), *itAsks)
        );
    }
    sellOrders_.emplace(orderId, std::forward<Order>(order));
}

void FeedHandler::cancelBuyOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose)
{
    auto itOrder = buyOrders_.find(orderId);
    if (unlikely(itOrder == buyOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Unknown buy orderId [" << orderId << "], cancel order impossible" << std::endl;
        ++errors.cancelsWithUnknownOrderId;
        return;
    }
    if (unlikely(getQty(itOrder->second) != getQty(order) || getPrice(itOrder->second) != getPrice(order)))
    {
        if (verbose > 0) std::cerr << "Found buy orderId [" << orderId << "] but order info differs, cancel order rejected" << std::endl;
        ++errors.cancelsNotMatchedQtyOrPrice;
        return;
    }
    
    auto itBids = std::lower_bound(bids_.begin(), bids_.end(), getPrice(order), 
        [](Limit& l, Price p) -> bool
        {
            return (getPrice(l) > p);
        });
    if (likely(itBids != bids_.end() && getPrice(*itBids) == getPrice(order)))
    {
        if (unlikely(getQty(*itBids) < getQty(order)))
        {
            if (verbose > 0) std::cerr << "Unexpected issue with buy orderId [" << orderId 
                << "] but order qty upper than bid qty, cancel order aborted" << std::endl;
            ++errors.cancelsLimitQtyTooLow;
            return;
        }
        getQty(*itBids) -= getQty(order);
        if (getQty(*itBids) == 0)
        {
            queue_.push_back(
                Data(static_cast<char>(Parser::Action::CANCEL), static_cast<char>(Parser::Side::BUY), 
                     static_cast<unsigned int>(itBids-bids_.begin()))
            );
            bids_.erase(itBids);
        }
        else
        {
            queue_.push_back(
                Data(static_cast<char>(Parser::Action::MODIFY), static_cast<char>(Parser::Side::BUY), 
                     static_cast<unsigned int>(itBids-bids_.begin()), *itBids)
            );
        }
    }
    else
    {
        if (verbose > 0) std::cerr << "Limit not found for orderId [" << orderId << "], cancel order failed" << std::endl;
        ++errors.cancelsLimitNotFound;
        return;
    }
    
    buyOrders_.erase(itOrder);
}

void FeedHandler::cancelSellOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose)
{
    auto itOrder = sellOrders_.find(orderId);
    if (unlikely(itOrder == sellOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Unknown sell orderId [" << orderId << "], cancel order impossible" << std::endl;
        ++errors.cancelsWithUnknownOrderId;
        return;
    }
    if (unlikely(getQty(itOrder->second) != getQty(order) || getPrice(itOrder->second) != getPrice(order)))
    {
        if (verbose > 0) std::cerr << "Found sell orderId [" << orderId << "] but order info differs, cancel order rejected" << std::endl;
        ++errors.cancelsNotMatchedQtyOrPrice;
        return;
    }
    
    auto itAsks = std::lower_bound(asks_.begin(), asks_.end(), getPrice(order), 
        [](Limit& l, Price p) -> bool
        {
            return (getPrice(l) < p);
        });
    if (likely(itAsks != asks_.end() && getPrice(*itAsks) == getPrice(order)))
    {
        if (unlikely(getQty(*itAsks) < getQty(order)))
        {
            if (verbose > 0) std::cerr << "Unexpected issue with sell orderId [" << orderId 
                << "] but order qty upper than ask qty, cancel order aborted" << std::endl;
            ++errors.cancelsLimitQtyTooLow;
            return;
        }
        getQty(*itAsks) -= getQty(order);
        if (getQty(*itAsks) == 0)
        {
            queue_.push_back(
                Data(static_cast<char>(Parser::Action::CANCEL), static_cast<char>(Parser::Side::SELL), 
                     static_cast<unsigned int>(itAsks-asks_.begin()))
            );
            asks_.erase(itAsks);
        }
        else
        {
            queue_.push_back(
                Data(static_cast<char>(Parser::Action::MODIFY), static_cast<char>(Parser::Side::SELL), 
                     static_cast<unsigned int>(itAsks-asks_.begin()), *itAsks)
            );
        }
    }
    else
    {
        if (verbose > 0) std::cerr << "Limit not found for orderId [" << orderId << "], cancel order failed" << std::endl;
        ++errors.cancelsLimitNotFound;
        return;
    }
    
    sellOrders_.erase(itOrder);
}

void FeedHandler::modifyBuyOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose)
{
    auto itOrder = buyOrders_.find(orderId);
    if (unlikely(itOrder == buyOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Unknown buy orderId [" << orderId << "], modify order impossible" << std::endl;
        ++errors.modifiesWithUnknownOrderId;
        return;
    }
    if (unlikely(getPrice(itOrder->second) != getPrice(order)))
    {
        if (verbose > 0) std::cerr << "Found buy orderId [" << orderId << "] but price differs, modify order rejected" << std::endl;
        ++errors.modifiesNotMatchedPrice;
        return;
    }

    auto itBids = std::lower_bound(bids_.begin(), bids_.end(), getPrice(order), 
        [](Limit& l, Price p) -> bool
        {
            return (getPrice(l) > p);
        });
    if (likely(itBids != bids_.end() && getPrice(*itBids) == getPrice(order)))
    {
        if (unlikely(getQty(*itBids) < getQty(itOrder->second)))
        {
            if (verbose > 0) std::cerr << "Unexpected issue with buy orderId [" << orderId 
                << "] but order qty upper than bid qty, modify order aborted" << std::endl;
            ++errors.modifiesLimitQtyTooLow;
            return;
        }
        getQty(*itBids) -= getQty(itOrder->second);
        getQty(*itBids) += getQty(order);
        if (unlikely(getQty(*itBids) == 0))
        {
            queue_.push_back(
                Data(static_cast<char>(Parser::Action::CANCEL), static_cast<char>(Parser::Side::BUY), 
                     static_cast<unsigned int>(itBids-bids_.begin()))
            );
            bids_.erase(itBids);
        }
        else
        {
            queue_.push_back(
                Data(static_cast<char>(Parser::Action::MODIFY), static_cast<char>(Parser::Side::BUY), 
                     static_cast<unsigned int>(itBids-bids_.begin()), *itBids)
            );
        }
    }
    else
    {
        if (verbose > 0) std::cerr << "Limit not found for orderId [" << orderId << "], modify order failed" << std::endl;
        ++errors.modifiesLimitNotFound;
        return;
    }

    itOrder->second = std::forward<Order>(order);
}

void FeedHandler::modifySellOrder(OrderId orderId, Order&& order, Errors& errors, const int verbose)
{
    auto itOrder = sellOrders_.find(orderId);
    if (unlikely(itOrder == sellOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Unknown sell orderId [" << orderId << "], modify order impossible" << std::endl;
        ++errors.modifiesWithUnknownOrderId;
        return;
    }
    if (unlikely(getPrice(itOrder->second) != getPrice(order)))
    {
        if (verbose > 0) std::cerr << "Found sell orderId [" << orderId << "] but price differs, modify order rejected" << std::endl;
        ++errors.modifiesNotMatchedPrice;
        return;
    }
    
    auto itAsks = std::lower_bound(asks_.begin(), asks_.end(), getPrice(order), 
        [](Limit& l, Price p) -> bool
        {
            return (getPrice(l) < p);
        });
    if (likely(itAsks != asks_.end() && getPrice(*itAsks) == getPrice(order)))
    {
        if (unlikely(getQty(*itAsks) < getQty(itOrder->second)))
        {
            if (verbose > 0) std::cerr << "Unexpected issue with sell orderId [" << orderId 
                << "] but order qty upper than ask qty, modify order aborted" << std::endl;
            ++errors.modifiesLimitQtyTooLow;
            return;
        }
        getQty(*itAsks) -= getQty(itOrder->second);
        getQty(*itAsks) += getQty(order);
        if (unlikely(getQty(*itAsks) == 0))
        {
            queue_.push_back(
                Data(static_cast<char>(Parser::Action::CANCEL), static_cast<char>(Parser::Side::SELL), 
                     static_cast<unsigned int>(itAsks-asks_.begin()))
            );
            asks_.erase(itAsks);
        }
        else
        {
            queue_.push_back(
                Data(static_cast<char>(Parser::Action::MODIFY), static_cast<char>(Parser::Side::SELL), 
                     static_cast<unsigned int>(itAsks-asks_.begin()), *itAsks)
            );
        }
    }
    else
    {
        if (verbose > 0) std::cerr << "Limit not found for orderId [" << orderId << "], modify order failed" << std::endl;
        ++errors.modifiesLimitNotFound;
        return;
    }

    itOrder->second = std::forward<Order>(order);
}

