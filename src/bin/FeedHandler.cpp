#include "FeedHandler.h"

#include "Parser.h"
#include "StrStream.h"

bool FeedHandler::processMessage(const std::string& line, Errors& errors, const int verbose)
{
//    std::cout << line << std::endl;  
    Parser p;
    if (!p.parse(line, errors, verbose)) return false;
    switch(p.getAction())
    {
    case static_cast<char>(Parser::Action::ADD):
        if (p.getSide() == static_cast<char>(Parser::Side::BUY))
        {
            return newBuyOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
        } 
        else
        {
            return newSellOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
        }
    case static_cast<char>(Parser::Action::CANCEL):
        if (p.getSide() == static_cast<char>(Parser::Side::BUY))
        {
            return cancelBuyOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
        } 
        else
        {
            return cancelSellOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
        }
    case static_cast<char>(Parser::Action::MODIFY):
        if (p.getSide() == static_cast<char>(Parser::Side::BUY))
        {
            return modifyBuyOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
        } 
        else
        {
            return modifySellOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
        }
    case static_cast<char>(Parser::Action::TRADE):
        break;
    }
    return false;
}

void FeedHandler::printMidQuotes(std::ostream& os) const
{
    if (unlikely(bids_.begin() == bids_.end() || asks_.begin() == asks_.end()))
    {
        os << "NAN" << std::endl;
        return;
    }
    StrStream strstream;
    Price midQuote = (getPrice(*bids_.begin())+getPrice(*asks_.begin()))/2;
    strstream << midQuote << '\n';
    os << strstream.c_str();
}




void FeedHandler::printErrors(std::ostream& os, Errors& errors, const int verbose)
{
    StrStream strstream;    
    strstream << "Summary:";
    
    // Parsing (not really errors)
    if (unlikely(errors.commentedLines))
    {
        strstream << "\n [" << errors.commentedLines << "] commented lines";
    }
    if (unlikely(errors.blankLines))
    {
        strstream << "\n [" << errors.blankLines << "] blank lines";
    }

    if (unlikely(errors.nbErrors() > 0))
    {
        // Parsing
        if (unlikely(errors.corruptedMessages))
        {
            strstream << "\n [" << errors.corruptedMessages << "] corrupted messages";
        }
        if (unlikely(errors.IncompleteMessages))
        {
            strstream << "\n [" << errors.IncompleteMessages << "] incomplete messages";
        }
        if (unlikely(errors.wrongActions))
        {
            strstream << "\n [" << errors.wrongActions << "] wrong actions";
        }
        if (unlikely(errors.wrongSides))
        {
            strstream << "\n [" << errors.wrongSides << "] wrong sides";
        }
        if (unlikely(errors.negativeOrderIds))
        {
            strstream << "\n [" << errors.negativeOrderIds << "] negative orderIds";
        }
        if (unlikely(errors.negativeQuantities))
        {
            strstream << "\n [" << errors.negativeQuantities << "] negative quantities";
        }
        if (unlikely(errors.negativePrices))
        {
            strstream << "\n [" << errors.negativePrices << "] negative prices";
        }
        if (unlikely(errors.missingActions))
        {
            strstream << "\n [" << errors.missingActions << "] missing actions";
        }
        if (unlikely(errors.missingOrderIds))
        {
            strstream << "\n [" << errors.missingOrderIds << "] missing orderIds";
        }
        if (unlikely(errors.missingSides))
        {
            strstream << "\n [" << errors.missingSides << "] missing sides";
        }
        if (unlikely(errors.missingQuantities))
        {
            strstream << "\n [" << errors.missingQuantities << "] missing quantities";
        }
        if (unlikely(errors.missingPrices))
        {
            strstream << "\n [" << errors.missingPrices << "] missing prices";
        }
        if (unlikely(errors.zeroOrderIds))
        {
            strstream << "\n [" << errors.zeroOrderIds << "] zero orderIds";
        }
        if (unlikely(errors.zeroQuantities))
        {
            strstream << "\n [" << errors.zeroQuantities << "] zero quantities";
        }
        if (unlikely(errors.zeroPrices))
        {
            strstream << "\n [" << errors.zeroPrices << "] zero prices";
        }
        if (unlikely(errors.outOfBoundsOrderIds))
        {
            strstream << "\n [" << errors.outOfBoundsOrderIds << "] out of bounds orderIds";
        }
        if (unlikely(errors.outOfBoundsQuantities))
        {
            strstream << "\n [" << errors.outOfBoundsQuantities << "] out of bounds quantities";
        }
        if (unlikely(errors.outOfBoundsPrices))
        {
            strstream << "\n [" << errors.outOfBoundsPrices << "] out of bounds prices";
        }
        
        // Order Management
        if (unlikely(errors.duplicateOrderIds))
        {
            strstream << "\n [" << errors.duplicateOrderIds << "] duplicate OrderIds";
        }
        if (unlikely(errors.modifiesWithUnknownOrderId))
        {
            strstream << "\n [" << errors.modifiesWithUnknownOrderId << "] modifies with unknown OrderIds";
        }
        if (unlikely(errors.cancelsWithUnknownOrderId))
        {
            strstream << "\n [" << errors.cancelsWithUnknownOrderId << "] cancels with unknown OrderIds";
        }
        if (unlikely(errors.bestBidEqualOrUpperThanBestAsk))
        {
            strstream << "\n [" << errors.bestBidEqualOrUpperThanBestAsk << "] best bid equal or upper than best ask";
        }
    }        
    else
    {
        strstream << " no error found";
    }
    strstream << '\n';
    if (unlikely(verbose))
        strstream << "Summary length: " << strstream.length() << '\n';
    os << strstream.c_str();
}

bool FeedHandler::newBuyOrder(OrderId orderId, FeedHandler::Order&& order, Errors& errors, const int verbose)
{
    if (unlikely(buyOrders_.find(orderId) != buyOrders_.end() || 
                 sellOrders_.find(orderId) != sellOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Duplicate orderId [" << orderId << "], new order rejected" << std::endl;
        ++errors.duplicateOrderIds;
        return false;
    }
    auto itBids = std::lower_bound(bids_.begin(), bids_.end(), getPrice(order), 
        [](Order& o, Price p) -> bool
        {
            return (getPrice(o) > p);
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

bool FeedHandler::newSellOrder(OrderId orderId, FeedHandler::Order&& order, Errors& errors, const int verbose)
{
    if (unlikely(buyOrders_.find(orderId) != buyOrders_.end() || 
                 sellOrders_.find(orderId) != sellOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Duplicate orderId [" << orderId << "], new order rejected" << std::endl;
        ++errors.duplicateOrderIds;
        return false;
    }
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

bool FeedHandler::cancelBuyOrder(OrderId orderId, FeedHandler::Order&& order, Errors& errors, const int verbose)
{
    auto itOrder = buyOrders_.find(orderId);
    if (unlikely(itOrder == buyOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Unknown buy orderId [" << orderId << "], cancel order impossible" << std::endl;
        ++errors.cancelsWithUnknownOrderId;
        return false;
    }
    if (unlikely(getQty(itOrder->second) != getQty(order) || getPrice(itOrder->second) != getPrice(order)))
    {
        if (verbose > 0) std::cerr << "found buy orderId [" << orderId << "] but order info differs, cancel order rejected" << std::endl;
        return false;
    }
    
    auto ret = true;
    auto itBids = std::lower_bound(bids_.begin(), bids_.end(), getPrice(order), 
        [](Order& o, Price p) -> bool
        {
            return (getPrice(o) > p);
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
        if (verbose > 0) std::cerr << "Limit not found for orderId [" << orderId << "], cancel order failed" << std::endl;
        ret = false;
    }
    
    buyOrders_.erase(itOrder);
    return ret;
}

bool FeedHandler::cancelSellOrder(OrderId orderId, FeedHandler::Order&& order, Errors& errors, const int verbose)
{
    auto itOrder = sellOrders_.find(orderId);
    if (unlikely(itOrder == sellOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Unknown sell orderId [" << orderId << "], cancel order impossible" << std::endl;
        ++errors.cancelsWithUnknownOrderId;
        return false;
    }
    if (unlikely(getQty(itOrder->second) != getQty(order) || getPrice(itOrder->second) != getPrice(order)))
    {
        if (verbose > 0) std::cerr << "found sell orderId [" << orderId << "] but order info differs, cancel order rejected" << std::endl;
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
        if (verbose > 0) std::cerr << "Limit not found for orderId [" << orderId << "], cancel order failed" << std::endl;
        ret = false;
    }
    
    sellOrders_.erase(itOrder);
    return ret;
}

bool FeedHandler::modifyBuyOrder(OrderId orderId, FeedHandler::Order&& order, Errors& errors, const int verbose)
{
    auto itOrder = buyOrders_.find(orderId);
    if (unlikely(itOrder == buyOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Unknown buy orderId [" << orderId << "], modify order impossible" << std::endl;
        ++errors.modifiesWithUnknownOrderId;
        return false;
    }
    if (unlikely(getPrice(itOrder->second) != getPrice(order)))
    {
        if (verbose > 0) std::cerr << "found buy orderId [" << orderId << "] but price differs, modify order rejected" << std::endl;
        return false;
    }

    auto ret = true;
    auto itBids = std::lower_bound(bids_.begin(), bids_.end(), getPrice(order), 
        [](Order& o, Price p) -> bool
        {
            return (getPrice(o) > p);
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
        if (verbose > 0) std::cerr << "Limit not found for orderId [" << orderId << "], modify order failed" << std::endl;
        ret = false;
    }

    itOrder->second = std::move(order);
    return true;
}

bool FeedHandler::modifySellOrder(OrderId orderId, FeedHandler::Order&& order, Errors& errors, const int verbose)
{
    auto itOrder = sellOrders_.find(orderId);
    if (unlikely(itOrder == sellOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Unknown sell orderId [" << orderId << "], modify order impossible" << std::endl;
        ++errors.modifiesWithUnknownOrderId;
        return false;
    }
    if (unlikely(getPrice(itOrder->second) != getPrice(order)))
    {
        if (verbose > 0) std::cerr << "found sell orderId [" << orderId << "] but price differs, modify order rejected" << std::endl;
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
        if (verbose > 0) std::cerr << "Limit not found for orderId [" << orderId << "], modify order failed" << std::endl;
        ret = false;
    }

    itOrder->second = std::move(order);
    return true;
}

