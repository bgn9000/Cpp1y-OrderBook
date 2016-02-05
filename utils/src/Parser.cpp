#include "Parser.h"

#include "Decoder.h"
#include "StrStream.h"

// action,orderid,side,quantity,price
// action = A (add), X (remove), M (modify)
// side = B (buy), S (sell)
// if action = T (Trade) : action,quantity,price
bool Parser::parse(const char* str, size_t len, Errors& errors, const int verbose)
{
    auto i = 0UL;
    
    auto firstField = [&]() -> bool
    {
        for (; i < len; ++i)
        {
            if (' ' == str[i]) continue;
            if (unlikely('/' == str[i]))
            {
                if (len > i+1 && '/' == str[i+1])
                {
                    if (verbose > 1) std::cerr << "Line is comment in [" << str << "]" << std::endl;
                    ++errors.commentedLines;
                    return false;
                }
                if (verbose > 0) std::cerr << "Bad comment in [" << str << "]" << std::endl;
                ++errors.corruptedMessages;
                return false;
            }
            if (unlikely(verbose > 2)) std::cerr << "firstField true" << std::endl;
            return true;
        }
        if (verbose > 1) std::cerr << "Blank line : " << str << std::endl;
        ++errors.blankLines;
        return false;
    };

    auto nextField = [&]() -> bool
    {
        for (; i < len; ++i)
        {
            if (likely(',' == str[i])) 
            { 
                ++i;
                if (unlikely(verbose > 2)) std::cerr << "nextField true" << std::endl;
                return true;
            }
            else if (unlikely(' ' != str[i]))
            {
                if (verbose > 0) std::cerr << "Bad formatted order info in [" << str << "]" << std::endl;
                ++errors.corruptedMessages;
                return false;
            }
        }
        if (verbose > 0) std::cerr << "Incomplete message in [" << str << "]" << std::endl;
        ++errors.IncompleteMessages;
        return false;
    };
    
    auto extractAction = [&]() -> bool
    {
        for (; i < len; ++i)
        {
            if (likely(' ' != str[i]))
            {
                if (unlikely(',' == str[i])) break;
                action_ = str[i];
                if (unlikely(static_cast<char>(Action::ADD) != action_ && 
                             static_cast<char>(Action::CANCEL) != action_ && 
                             static_cast<char>(Action::MODIFY) != action_ &&
                             static_cast<char>(Action::TRADE) != action_))
                {
                    if (verbose > 0) std::cerr << "Expected valid action in [" << str << "]" << std::endl;
                    ++errors.wrongActions;
                    return false;
                }
                ++i;
                if (unlikely(verbose > 2)) std::cerr << "extractAction true" << std::endl;
                return true;
            }
        }
        if (verbose > 0) std::cerr << "Missing action [" << str << "]" << std::endl;
        ++errors.missingActions;
        return false;
    };
    
    auto extractOrderId = [&]() -> bool
    {
        if (unlikely(static_cast<char>(Action::TRADE) == action_)) { --i; return true; }
        auto j = i, start = 0UL, end = 0UL;
        for (; j < len; ++j)
        {
            if (likely(std::isdigit(str[j])))
            {
                if (!start) start = j;
                continue;
            }
            if (unlikely(' ' != str[j] && ',' != str[j]))
            {
                if ('-' == str[j] && !start)
                {
                    if (verbose > 0) std::cerr << "Expected positive orderId in [" << str << "]" << std::endl;
                    ++errors.negativeOrderIds;
                }
                else
                {
                    if (verbose > 0) std::cerr << "Expected valid orderId in [" << str << "]" << std::endl;
                    ++errors.corruptedMessages;
                }
                return false;
            }
            if (likely(start))
            {
                end = j;
                break;
            }
            if (',' == str[j])
            {
                if (verbose > 0) std::cerr << "Missing orderId in [" << str << "]" << std::endl;
                ++errors.missingOrderIds;
                return false;
            }
        }
        if (unlikely(j == len))
        {
            if (verbose > 0) std::cerr << "Incomplete message in [" << str << "]" << std::endl;
            ++errors.IncompleteMessages;
            return false;
        }
        i = j;
        const long dataLen = end - start;
        if (likely(dataLen > 0))
        {
            if (unlikely(dataLen > nbCharOfOrderId))
            {
                if (verbose > 0) std::cerr << "Expected orderId less than 1 billion in [" << str << "]" << std::endl;
                ++errors.outOfBoundsOrderIds;
                return false;
            }
            orderId_ = Decoder::retreive_unsigned_integer<OrderId>(&str[start], dataLen);
            if (unlikely(0 == orderId_))
            {
                if (verbose > 0) std::cerr << "Expected non zero orderId in [" << str << "]" << std::endl;
                ++errors.zeroOrderIds;
                return false;
            }
            if (unlikely(verbose > 2)) std::cerr << "extractOrderId true" << std::endl;
            return true;
        }
        if (verbose > 0) std::cerr << "Missing orderId in [" << str << "]" << std::endl;
        ++errors.missingOrderIds;
        return false;
    };
    
    auto extractSide = [&]() -> bool
    {
        if (unlikely(static_cast<char>(Action::TRADE) == action_)) { --i; return true; }
        for (; i < len; ++i)
        {
            if (likely(' ' != str[i]))
            {
                if (unlikely(',' == str[i])) break;
                side_ = str[i];
                if (unlikely(static_cast<char>(Side::BUY) != side_ && 
                             static_cast<char>(Side::SELL) != side_))
                {
                    if (verbose > 0) std::cerr << "Expected valid side in [" << str << "]" << std::endl;
                    ++errors.wrongSides;
                    return false;
                }
                ++i;
                if (unlikely(verbose > 2)) std::cerr << "extractSide true" << std::endl;
                return true;
            }
        }
        if (unlikely(i == len))
        {
            if (verbose > 0) std::cerr << "Incomplete message in [" << str << "]" << std::endl;
            ++errors.IncompleteMessages;
            return false;
        }
        if (verbose > 0) std::cerr << "Missing side [" << str << "]" << std::endl;
        ++errors.missingSides;
        return false;
    };
    
    auto extractQty = [&]() -> bool
    {
        auto j = i, start = 0UL, end = 0UL;
        for (; j < len; ++j)
        {
            if (likely(std::isdigit(str[j])))
            {
                if (!start) start = j;
                continue;
            }
            if (unlikely(' ' != str[j] && ',' != str[j]))
            {
                if ('-' == str[j] && !start)
                {
                    if (verbose > 0) std::cerr << "Expected positive qty in [" << str << "]" << std::endl;
                    ++errors.negativeQuantities;
                }
                else
                {
                    if (verbose > 0) std::cerr << "Expected valid qty in [" << str << "]" << std::endl;
                    ++errors.corruptedMessages;
                }
                return false;
            }
            if (likely(start))
            {
                end = j;
                break;
            }
            if (',' == str[j])
            {
                if (verbose > 0) std::cerr << "Missing qty in [" << str << "]" << std::endl;
                ++errors.missingQuantities;
                return false;
            }
        }
        if (unlikely(j == len))
        {
            if (verbose > 0) std::cerr << "Incomplete message in [" << str << "]" << std::endl;
            ++errors.IncompleteMessages;
            return false;
        }
        i = j;
        const long dataLen = end - start;
        if (likely(dataLen > 0))
        {
            if (unlikely(dataLen > nbCharOfOrderQty))
            {
                if (verbose > 0) std::cerr << "Expected qty less than 1 million in [" << str << "]" << std::endl;
                ++errors.outOfBoundsQuantities;
                return false;
            }
            qty_ = Decoder::retreive_unsigned_integer<Quantity>(&str[start], dataLen);
            if (unlikely(0 == qty_))
            {
                if (verbose > 0) std::cerr << "Expected non zero qty in [" << str << "]" << std::endl;
                ++errors.zeroQuantities;
                return false;
            }
            if (unlikely(verbose > 2)) std::cerr << "extractQty true" << std::endl;
            return true;
        }
        if (verbose > 0) std::cerr << "Missing qty in [" << str << "]" << std::endl;
        ++errors.missingQuantities;
        return false;
    };
    
    auto extractPrice = [&]() -> bool
    {
        auto j = i, start = 0UL, end = 0UL, dot = 0UL;
        for (; j < len; ++j)
        {
            if (likely(std::isdigit(str[j])))
            {
                if (unlikely(!start)) start = j;
                continue;
            }
            if ('.' == str[j])
            {
                if (unlikely(dot || !start))
                {
                    if (verbose > 0) std::cerr << "Expected valid price in [" << str << "]" << std::endl;
                    ++errors.corruptedMessages;
                    return false;
                }
                dot = j;
                continue;
            }
            if (unlikely(str[j] != ' ' && str[j] != ',' && str[j] != '/'))
            {
                if ('-' == str[j] && !start)
                {
                    if (verbose > 0) std::cerr << "Expected positive price in [" << str << "]" << std::endl;
                    ++errors.negativePrices;
                }
                else
                {
                    if (verbose > 0) std::cerr << "Expected valid price in [" << str << "]" << std::endl;
                    ++errors.corruptedMessages;
                }
                return false;
            }
            if (likely(start))
            {
                end = j;
                break;
            }
            if (',' == str[j])
            {
                if (verbose > 0) std::cerr << "Missing price in [" << str << "]" << std::endl;
                ++errors.missingPrices;
                return false;
            }
        }
        if (likely((j == len && start) || end > start))
        {
            if (likely(j == len)) end = len;
            if (!dot)
            {
                if (unlikely(end - start > nbCharOfOrderPrice))
                {
                    if (verbose > 0) std::cerr << "Expected price less than 1 billion in [" << str << "]" << std::endl;
                    ++errors.outOfBoundsPrices;
                    return false;
                }
                price_ = Decoder::retreive_unsigned_float<Price>(&str[start], end-start);
            }
            else
            {
                if (unlikely(dot - start > nbCharOfOrderPrice))
                {
                    if (verbose > 0) std::cerr << "Expected price less than 1 billion in [" << str << "]" << std::endl;
                    ++errors.outOfBoundsPrices;
                    return false;
                }
                if (likely(end - (dot + 1) <= nbCharOfPricePrecision))
                {
                    price_ = Decoder::retreive_unsigned_float<Price>(&str[start], end-start);
                }
                else
                {
                    price_ = Decoder::retreive_unsigned_float<Price>(&str[start], dot+1+nbCharOfPricePrecision-start);
                }
            }
            if (unlikely(0.0 == price_))
            {
                if (verbose > 0) std::cerr << "Expected non zero price in [" << str << "]" << std::endl;
                ++errors.zeroPrices;
                return false;
            }
            if (unlikely(verbose > 2)) std::cerr << "extractPrice true : " << str << std::endl;
            return true;
        }
        if (verbose > 0) std::cerr << "Missing price in [" << str << "]" << std::endl;
        ++errors.missingPrices;
        return false;
    };
    
    return (firstField()      &&
            extractAction()   && nextField() &&
            extractOrderId()  && nextField() &&
            extractSide()     && nextField() &&
            extractQty()      && nextField() &&
            extractPrice());
}

