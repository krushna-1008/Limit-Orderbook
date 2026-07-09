#pragma once

#include "OrderModify.hpp"
#include "Trade.hpp"

#include <functional>
#include <list>
#include <map>
#include <unordered_map>

using OrderPointers = std::list<OrderPointer>;

class OrderBookLevelInfos
{
public:
    OrderBookLevelInfos(const LevelInfos& bids, const LevelInfos& asks);

    const LevelInfos& GetBids() const;
    const LevelInfos& GetAsks() const;

private:
    LevelInfos bids_;
    LevelInfos asks_;
};

class OrderBook
{
public:
    Trades AddOrder(OrderPointer order);
    void CancelOrder(OrderId orderId);
    Trades ModifyOrder(OrderModify order);

    std::size_t Size() const;
    OrderBookLevelInfos GetOrderInfo() const;

private:
    struct OrderEntry
    {
        OrderPointer order{nullptr};
        OrderPointers::iterator location;
    };

    bool ContainsOrder(OrderId orderId) const;
    bool CanMatch(Side side, Price price) const;
    Trades MatchOrders();
    void CancelFillAndKillRemainders();

    std::map<Price, OrderPointers, std::greater<Price>> bids_;
    std::map<Price, OrderPointers, std::less<Price>> asks_;
    std::unordered_map<OrderId, OrderEntry> orders_;
};

