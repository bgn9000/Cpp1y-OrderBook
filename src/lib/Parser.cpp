#include "Parser.h"

#include "Decoder.h"

// action,orderid,side,quantity,price
// action = A (add), X (remove), M (modify)
// side = B (buy), S (sell)
// if action = T (Trade) : action,quantity,price
bool Parser::parse(const std::string& str, const int verbose)
{
    const auto len = str.size();
    auto i = 0U;
    
    auto firstField = [&]() -> bool
    {
        for (; i < len; ++i)
        {
            if (' ' == str[i]) continue;
            if (unlikely('/' == str[i]))
            {
                if (len > i+1 && '/' == str[i+1]) return false;
                if (verbose > 0) std::cerr << "Bad comment in [" << str << "]" << std::endl;
                return false;
            }
            return true;
        }
        if (verbose > 1) std::cerr << "firstField false : " << str << std::endl;
        return false;
    };

    auto nextField = [&]() -> bool
    {
        for (; i < len; ++i)
        {
            if (likely(',' == str[i])) 
            { 
                ++i;
                if (verbose > 2) std::cerr << "nextField true" << std::endl;
                return true;
            }
            else if (unlikely(' ' != str[i]))
            {
                if (verbose > 0) std::cerr << "Bad formatted order info in [" << str << "]" << std::endl;
                if (verbose > 1) std::cerr << "nextField false : " << str << std::endl;
                return false;
            }
        }
        if (verbose > 0) std::cerr << "Incomplete order info in [" << str << "]" << std::endl;
        if (verbose > 1) std::cerr << "nextField false : " << str << std::endl;
        return false;
    };
    
    auto extractAction = [&]() -> bool
    {
        for (; i < len; ++i)
        {
            if (likely(' ' != str[i]))
            {
                action_ = str[i];
                if (unlikely(static_cast<char>(Action::ADD) != action_ && 
                             static_cast<char>(Action::CANCEL) != action_ && 
                             static_cast<char>(Action::MODIFY) != action_ &&
                             static_cast<char>(Action::TRADE) != action_))
                {
                    if (verbose > 0) std::cerr << "Expected valid action in [" << str << "]" << std::endl;
                    return false;
                }
                ++i;
                if (verbose > 2) std::cerr << "extractAction true" << std::endl;
                return true;
            }
        }
        if (verbose > 1) std::cerr << "extractAction false : " << str << std::endl;
        return false;
    };
    
    auto extractOrderId = [&]() -> bool
    {
        if (unlikely(static_cast<char>(Action::TRADE) == action_)) { --i; return true; }
        auto j = i, start = 0U, end = 0U;
        for (; j < len; ++j)
        {
            if (likely(std::isdigit(str[j])))
            {
                if (!start) start = j;
                continue;
            }
            if (unlikely(' ' != str[j] && ',' != str[j]))
            {
                if (verbose > 0) std::cerr << "Expected valid orderId in [" << str << "]" << std::endl;
                return false;
            }
            if (likely(start))
            {
                end = j;
                break;
            }
            if (',' == str[j])
            {
                if (verbose > 0) std::cerr << "Expected valid orderId in [" << str << "]" << std::endl;
                return false;
            }
        }
        i = j;
        if (end > start)
        {
            orderId_ = Decoder::retreive_unsigned_integer<OrderId>(&str[start], end-start);
            if (verbose > 2) std::cerr << "extractOrderId true" << std::endl;
            return true;
        }
        if (verbose > 1) std::cerr << "extractOrderId false : " << str << std::endl;
        return false;
    };
    
    auto extractSide = [&]() -> bool
    {
        if (unlikely(static_cast<char>(Action::TRADE) == action_)) { --i; return true; }
        for (; i < len; ++i)
        {
            if (likely(' ' != str[i]))
            {
                side_ = str[i];
                if (unlikely(static_cast<char>(Side::BUY) != side_ && 
                             static_cast<char>(Side::SELL) != side_))
                {
                    if (verbose > 0) std::cerr << "Expected valid side in [" << str << "]" << std::endl;
                    return false;
                }
                ++i;
                if (verbose > 2) std::cerr << "extractSide true" << std::endl;
                return true;
            }
        }
        if (verbose > 1) std::cerr << "extractSide false : " << str << std::endl;
        return false;
    };
    
    auto extractQty = [&]() -> bool
    {
        auto j = i, start = 0U, end = 0U;
        for (; j < len; ++j)
        {
            if (likely(std::isdigit(str[j])))
            {
                if (!start) start = j;
                continue;
            }
            if (unlikely(' ' != str[j] && ',' != str[j]))
            {
                if (verbose > 0) std::cerr << "Expected valid qty in [" << str << "]" << std::endl;
                return false;
            }
            if (likely(start))
            {
                end = j;
                break;
            }
            if (',' == str[j])
            {
                if (verbose > 0) std::cerr << "Expected valid qty in [" << str << "]" << std::endl;
                return false;
            }
        }
        i = j;
        if (end > start)
        {
            qty_ = Decoder::retreive_unsigned_integer<Quantity>(&str[start], end-start);
            if (verbose > 2) std::cerr << "extractQty true" << std::endl;
            return true;
        }
        if (verbose > 1) std::cerr << "extractQty false : " << str << std::endl;
        return false;
    };
    
    auto extractPrice = [&]() -> bool
    {
        auto j = i, start = 0U, end = 0U;
        bool dot = false;
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
                    return false;
                }
                dot = true;
                continue;
            }
            if (unlikely(str[j] != ' ' && str[j] != ',' && str[j] != '/'))
            {
                if (verbose > 0) std::cerr << "Expected valid price in [" << str << "]" << std::endl;
                return false;
            }
            if (likely(start))
            {
                end = j;
                break;
            }
            if (',' == str[j])
            {
                if (verbose > 0) std::cerr << "Expected valid price in [" << str << "]" << std::endl;
                return false;
            }
        }
        if (j == len)
        {
            if (start) 
            {
                price_ = Decoder::retreive_float<Price>(&str[start], len-start);
                if (verbose > 2) std::cerr << "!!! extractPrice true : " << str << std::endl;
                return true;
            }
        }
        else if (end > start)
        {
            price_ = Decoder::retreive_float<Price>(&str[start], end-start);
            if (verbose > 2) std::cerr << "!!! extractPrice true : " << str << std::endl;
            return true;
        }
        if (verbose > 1) std::cerr << "extractPrice false : " << str << std::endl;
        return false;
    };
    
    return (firstField()      &&
            extractAction()   && nextField() &&
            extractOrderId()  && nextField() &&
            extractSide()     && nextField() &&
            extractQty()      && nextField() &&
            extractPrice());
}
