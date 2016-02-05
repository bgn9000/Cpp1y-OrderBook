#pragma once

#include "FeedHandler.h"

class Reporter
{    
public:
    Reporter() = default;
    ~Reporter() = default;
    Reporter(const Reporter&) = delete;
    Reporter& operator=(const Reporter&) = delete;
    
    bool processData(FeedHandler::Data&& data);

    void printCurrentOrderBook(std::ostream& os) const;
    void printMidQuotesAndTrades(std::ostream& os, Errors& errors);
    void printErrors(std::ostream& os, Errors& errors, const int verbose = 0);
  
protected:
    bool treatTrade(Trade&& newTrade);

    std::deque<Limit> bids_, asks_;
    Trade currentTrade_{0ULL, 0.0};
    bool receivedNewTrade_ = false;
    bool detectCross_ = false;
};

