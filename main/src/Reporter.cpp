#include "Reporter.h"

#include <utils/Parser.h>
#include <utils/StrStream.h>

bool Reporter::processData(FeedHandler::Data&& data)
{
    switch(data.action_)
    {
    case static_cast<char>(Parser::Action::ADD):
        switch(data.side_)
        {
        case static_cast<char>(Parser::Side::BUY):
            bids_.insert(bids_.begin()+data.pos_, std::move(data.limit_));
            break;
        case static_cast<char>(Parser::Side::SELL):
            asks_.insert(asks_.begin()+data.pos_, std::move(data.limit_));
            break;
        default:
            break;
        }
        break;
    case static_cast<char>(Parser::Action::CANCEL):
        switch(data.side_)
        {
        case static_cast<char>(Parser::Side::BUY):
            bids_.erase(bids_.begin()+data.pos_);
            break;
        case static_cast<char>(Parser::Side::SELL):
            asks_.erase(asks_.begin()+data.pos_);
            break;
        default:
            break;
        }
        break;
    case static_cast<char>(Parser::Action::MODIFY):
        switch(data.side_)
        {
        case static_cast<char>(Parser::Side::BUY):
            bids_[data.pos_] = std::move(data.limit_);
            break;
        case static_cast<char>(Parser::Side::SELL):
            asks_[data.pos_] = std::move(data.limit_);
            break;
        default:
            break;
        }
        break;
    case static_cast<char>(Parser::Action::TRADE):
        treatTrade(std::move(data.limit_));
        break;
    default: // would behave as a false end reached
    case 0: // only when no more data and dontSpin is true => stop at the end
        return false;
    }
    return true;
}

void Reporter::printMidQuotesAndTrades(std::ostream& os, Errors& errors)
{
    StrStream strstream;
    if (unlikely(bids_.begin() == bids_.end() || asks_.begin() == asks_.end()))
    {
        strstream << "NAN" << '\n';
    }
    else if (receivedNewTrade_)
    {
        strstream << getQty(currentTrade_) << '@' << getPrice(currentTrade_) << '\n';
        receivedNewTrade_ = false;
        detectCross_ = false;
    }
    else if (unlikely(getPrice(*bids_.begin()) >= getPrice(*asks_.begin())))
    {
        if (likely(!detectCross_)) detectCross_ = true;
        else
        {
            strstream << "Cross BID (" << getPrice(*bids_.begin()) <<  ")/ASK(" << getPrice(*asks_.begin()) << ')' << '\n';
            ++errors.bestBidEqualOrUpperThanBestAsk;
        }
    }
    else
    {
        Price midQuote = (getPrice(*bids_.begin())+getPrice(*asks_.begin()))/2;
        strstream << midQuote << '\n';
    }
    os.rdbuf()->sputn(strstream.c_str(), strstream.length());
    os.flush();
}

void Reporter::printCurrentOrderBook(std::ostream& os) const
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

void Reporter::printErrors(std::ostream& os, Errors& errors, const int verbose)
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
    
    auto nbErrors = errors.nbErrors();
    if (unlikely(nbErrors > 0))
    {
        strstream << "\nFound " << nbErrors << " error:";
        
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
        if (unlikely(errors.modifiesNotMatchedPrice))
        {
            strstream << "\n [" << errors.modifiesNotMatchedPrice << "] modifies with not matched order price";
        }
        if (unlikely(errors.cancelsWithUnknownOrderId))
        {
            strstream << "\n [" << errors.cancelsWithUnknownOrderId << "] cancels with unknown OrderIds";
        }
        if (unlikely(errors.cancelsNotMatchedQtyOrPrice))
        {
            strstream << "\n [" << errors.cancelsNotMatchedQtyOrPrice << "] cancels with not matched order price";
        }
        if (unlikely(errors.bestBidEqualOrUpperThanBestAsk))
        {
            strstream << "\n [" << errors.bestBidEqualOrUpperThanBestAsk << "] best bid equal or upper than best ask";
        }
    }        
    else
    {
        strstream << "\nNo error found";
    }
    
    auto nbCriticalErrors = errors.nbCriticalErrors();
    if (unlikely(nbCriticalErrors > 0))
    {
        strstream << "\nFound [" << nbCriticalErrors << "] critical error:";
        if (unlikely(errors.modifiesLimitQtyTooLow))
        {
            strstream << "\n [" << errors.modifiesLimitQtyTooLow << "] modifies with limit quantity too low";
        }
        if (unlikely(errors.modifiesLimitNotFound))
        {
            strstream << "\n [" << errors.modifiesLimitNotFound << "] modifies limit not found";
        }
        if (unlikely(errors.cancelsLimitQtyTooLow))
        {
            strstream << "\n [" << errors.cancelsLimitQtyTooLow << "] cancels with limit quantity too low";
        }
        if (unlikely(errors.cancelsLimitNotFound))
        {
            strstream << "\n [" << errors.cancelsLimitNotFound << "] cancels limit not found";
        }
    }
    else
    {
        strstream << "\nNo critical error found";
    }
    
    strstream << '\n';
    if (unlikely(verbose))
        strstream << "Summary length: " << strstream.length() << '\n';
    os.rdbuf()->sputn(strstream.c_str(), strstream.length());
    os.flush();
}

bool Reporter::treatTrade(Trade&& newTrade)
{
    receivedNewTrade_ = true;
    if (getPrice(newTrade) == getPrice(currentTrade_))
    {
        getQty(currentTrade_) += getQty(newTrade);
    }
    else
    {
        currentTrade_ = std::forward<Trade>(newTrade);
    }
    return true;
}

