#pragma once

#include "Types.hpp"

#include <vector>

struct TradeInfo
{
    OrderId orderId;
    Price price;
    Quantity quantity;
};

class Trade
{
public:
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade);

    const TradeInfo& GetBidTrade() const;
    const TradeInfo& GetAskTrade() const;

private:
    TradeInfo bidTrade_;
    TradeInfo askTrade_;
};

using Trades = std::vector<Trade>;

