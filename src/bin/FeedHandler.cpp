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
        switch(p.getSide())
        {
        case static_cast<char>(Parser::Side::BUY):
            return newBuyOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
        case static_cast<char>(Parser::Side::SELL):
            return newSellOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
        }
    case static_cast<char>(Parser::Action::CANCEL):
        switch(p.getSide())
        {
        case static_cast<char>(Parser::Side::BUY):
            return cancelBuyOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
        case static_cast<char>(Parser::Side::SELL):
            return cancelSellOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
        }
    case static_cast<char>(Parser::Action::MODIFY):
        switch(p.getSide())
        {
        case static_cast<char>(Parser::Side::BUY):
            return modifyBuyOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
        case static_cast<char>(Parser::Side::SELL):
            return modifySellOrder(p.getOrderId(), Order{p.getQty(), p.getPrice()}, errors, verbose);
        }
    case static_cast<char>(Parser::Action::TRADE):
        break;
    }
    return false;
}

void FeedHandler::printMidQuotes(std::ostream& os) const
{
    StrStream strstream;
    if (unlikely(bids_.begin() == bids_.end() || asks_.begin() == asks_.end()))
    {
        strstream << "NAN";
    }
    else if (unlikely(getPrice(*bids_.begin()) >= getPrice(*asks_.begin())))
    {
        strstream << "Cross BID (" << getPrice(*bids_.begin()) <<  ")/ASK(" << getPrice(*asks_.begin()) << ')';
    }
    else
    {
        Price midQuote = (getPrice(*bids_.begin())+getPrice(*asks_.begin()))/2;
        strstream << midQuote;
    }
    strstream << '\n';
    os.rdbuf()->sputn(strstream.c_str(), strstream.length());
    os.flush();
}

void FeedHandler::printCurrentOrderBook(std::ostream& os) const
{
    StrStream strstream;
    auto cap = strstream.capacity() - 128;
    const auto nbBids = bids_.size();
    const auto nbAsks = asks_.size();
    os << "Full Bids/Asks:\n";
    auto i = 0U;
    while (1)
    {
        StrStream strstream_tmp;
        if (i < nbBids)
        {
            Limit bid = bids_[i];
            strstream_tmp << i; 
            strstream_tmp.append(6, ' ');
            strstream_tmp << ": " << getQty(bid) << " @ " << getPrice(bid);
            strstream_tmp.append(40, ' ');
            if (i < nbAsks)
            {
                Limit ask = asks_[i];
                strstream_tmp << getQty(ask) << " @ " << getPrice(ask) << '\n';
            }
            else
            {
                strstream_tmp << "empty\n";
            }
        }
        else
        {
            strstream_tmp << i;
            strstream_tmp.append(6, ' ');
            strstream_tmp << ": empty";
            strstream_tmp.append(40, ' ');
            if (i < nbAsks)
            {
                Limit ask = asks_[i];
                strstream_tmp << getQty(ask) << " @ " << getPrice(ask) << '\n';
            }
            else
            {
                strstream << strstream_tmp;
                strstream << "empty\n";
                break;
            }
        }

        if (strstream.length() + strstream_tmp.length() > cap)
        {
            os.rdbuf()->sputn(strstream.c_str(), strstream.length());
            strstream.clear();
        }
        strstream << strstream_tmp;
        ++i;
    }
    os.rdbuf()->sputn(strstream.c_str(), strstream.length());
    os.flush();
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
        strstream << "\n no error found";
    }
    strstream << '\n';
    if (unlikely(verbose))
        strstream << "Summary length: " << strstream.length() << '\n';
    os.rdbuf()->sputn(strstream.c_str(), strstream.length());
    os.flush();
}

bool FeedHandler::newBuyOrder(OrderId orderId, FeedHandler::Order&& order, Errors& errors, const int verbose)
{
    if (unlikely(buyOrders_.find(orderId) != buyOrders_.end() || 
                 sellOrders_.find(orderId) != sellOrders_.end()))
    {
        if (verbose > 0) std::cerr << "Duplicate buy orderId [" << orderId << "], new order rejected" << std::endl;
        ++errors.duplicateOrderIds;
        return false;
    }
    auto itBids = std::lower_bound(bids_.begin(), bids_.end(), getPrice(order), 
        [](Limit& l, Price p) -> bool
        {
            return (getPrice(l) > p);
        });
    if (getPrice(*itBids) == getPrice(order))
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
        if (verbose > 0) std::cerr << "Duplicate sell orderId [" << orderId << "], new order rejected" << std::endl;
        ++errors.duplicateOrderIds;
        return false;
    }
    auto itAsks = std::lower_bound(asks_.begin(), asks_.end(), getPrice(order), 
        [](Limit& l, Price p) -> bool
        {
            return (getPrice(l) < p);
        });
    if (getPrice(*itAsks) == getPrice(order))
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
        if (verbose > 0) std::cerr << "Found buy orderId [" << orderId << "] but order info differs, cancel order rejected" << std::endl;
        return false;
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
            return false;
        }
        getQty(*itBids) -= getQty(order);
        if (getQty(*itBids) == 0)
        {
            bids_.erase(itBids);
        }
    }
    else
    {
        if (verbose > 0) std::cerr << "Limit not found for orderId [" << orderId << "], cancel order failed" << std::endl;
        return false;
    }
    
    buyOrders_.erase(itOrder);
    return true;
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
        if (verbose > 0) std::cerr << "Found sell orderId [" << orderId << "] but order info differs, cancel order rejected" << std::endl;
        return false;
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
            return false;
        }
        getQty(*itAsks) -= getQty(order);
        if (getQty(*itAsks) == 0)
        {
            asks_.erase(itAsks);
        }
    }
    else
    {
        if (verbose > 0) std::cerr << "Limit not found for orderId [" << orderId << "], cancel order failed" << std::endl;
        return false;
    }
    
    sellOrders_.erase(itOrder);
    return true;
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
        if (verbose > 0) std::cerr << "Found buy orderId [" << orderId << "] but price differs, modify order rejected" << std::endl;
        return false;
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
            return false;
        }
        getQty(*itBids) -= getQty(itOrder->second);
        getQty(*itBids) += getQty(order);
        if (unlikely(getQty(*itBids) == 0))
        {
            bids_.erase(itBids);
        }
    }
    else
    {
        if (verbose > 0) std::cerr << "Limit not found for orderId [" << orderId << "], modify order failed" << std::endl;
        return false;
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
        if (verbose > 0) std::cerr << "Found sell orderId [" << orderId << "] but price differs, modify order rejected" << std::endl;
        return false;
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
            return false;
        }
        getQty(*itAsks) -= getQty(itOrder->second);
        getQty(*itAsks) += getQty(order);
        if (unlikely(getQty(*itAsks) == 0))
        {
            asks_.erase(itAsks);
        }
    }
    else
    {
        if (verbose > 0) std::cerr << "Limit not found for orderId [" << orderId << "], modify order failed" << std::endl;
        return false;
    }

    itOrder->second = std::move(order);
    return true;
}

