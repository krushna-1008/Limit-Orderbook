#include "OrderBook.hpp"

#include <algorithm>
#include <iterator>
#include <numeric>

OrderBookLevelInfos::OrderBookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
    : bids_{bids}, asks_{asks}
{
}

const LevelInfos& OrderBookLevelInfos::GetBids() const { return bids_; }
const LevelInfos& OrderBookLevelInfos::GetAsks() const { return asks_; }

bool OrderBook::ContainsOrder(OrderId orderId) const
{
    return orders_.find(orderId) != orders_.end();
}

bool OrderBook::CanMatch(Side side, Price price) const
{
    if (side == Side::Buy)
    {
        return !asks_.empty() && price >= asks_.begin()->first;
    }

    return !bids_.empty() && price <= bids_.begin()->first;
}

Trades OrderBook::AddOrder(OrderPointer order)
{
    if (!order || ContainsOrder(order->GetOrderId()))
    {
        return {};
    }

    if (order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice()))
    {
        return {};
    }

    OrderPointers::iterator location;
    if (order->GetSide() == Side::Buy)
    {
        auto& orders = bids_[order->GetPrice()];
        orders.push_back(order);
        location = std::prev(orders.end());
    }
    else
    {
        auto& orders = asks_[order->GetPrice()];
        orders.push_back(order);
        location = std::prev(orders.end());
    }

    orders_.insert({order->GetOrderId(), OrderEntry{order, location}});
    return MatchOrders();
}

void OrderBook::CancelOrder(OrderId orderId)
{
    if (!ContainsOrder(orderId))
    {
        return;
    }

    const auto [order, orderIterator] = orders_.at(orderId);
    orders_.erase(orderId);

    if (order->GetSide() == Side::Sell)
    {
        auto price = order->GetPrice();
        auto& orders = asks_.at(price);
        orders.erase(orderIterator);

        if (orders.empty())
        {
            asks_.erase(price);
        }
    }
    else
    {
        auto price = order->GetPrice();
        auto& orders = bids_.at(price);
        orders.erase(orderIterator);

        if (orders.empty())
        {
            bids_.erase(price);
        }
    }
}

Trades OrderBook::ModifyOrder(OrderModify order)
{
    if (!ContainsOrder(order.GetOrderId()))
    {
        return {};
    }

    const auto existingOrder = orders_.at(order.GetOrderId()).order;
    CancelOrder(order.GetOrderId());
    return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
}

std::size_t OrderBook::Size() const { return orders_.size(); }

OrderBookLevelInfos OrderBook::GetOrderInfo() const
{
    LevelInfos bidInfos;
    LevelInfos askInfos;
    bidInfos.reserve(bids_.size());
    askInfos.reserve(asks_.size());

    auto createLevelInfo = [](Price price, const OrderPointers& orders)
    {
        return LevelInfo{
            price,
            std::accumulate(orders.begin(), orders.end(), Quantity{0},
                            [](Quantity runningSum, const OrderPointer& order)
                            {
                                return runningSum + order->GetRemainingQuantity();
                            })};
    };

    for (const auto& [price, orders] : bids_)
    {
        bidInfos.push_back(createLevelInfo(price, orders));
    }

    for (const auto& [price, orders] : asks_)
    {
        askInfos.push_back(createLevelInfo(price, orders));
    }

    return OrderBookLevelInfos{bidInfos, askInfos};
}

Trades OrderBook::MatchOrders()
{
    Trades trades;
    trades.reserve(orders_.size());

    while (!bids_.empty() && !asks_.empty())
    {
        auto bidLevel = bids_.begin();
        auto askLevel = asks_.begin();

        if (bidLevel->first < askLevel->first)
        {
            break;
        }

        auto& bids = bidLevel->second;
        auto& asks = askLevel->second;

        while (!bids.empty() && !asks.empty())
        {
            auto bid = bids.front();
            auto ask = asks.front();
            const Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

            bid->Fill(quantity);
            ask->Fill(quantity);

            trades.push_back(Trade{
                TradeInfo{bid->GetOrderId(), bid->GetPrice(), quantity},
                TradeInfo{ask->GetOrderId(), ask->GetPrice(), quantity}});

            if (bid->IsFilled())
            {
                orders_.erase(bid->GetOrderId());
                bids.pop_front();
            }

            if (ask->IsFilled())
            {
                orders_.erase(ask->GetOrderId());
                asks.pop_front();
            }
        }

        if (bids.empty())
        {
            bids_.erase(bidLevel);
        }

        if (asks.empty())
        {
            asks_.erase(askLevel);
        }
    }

    CancelFillAndKillRemainders();
    return trades;
}

void OrderBook::CancelFillAndKillRemainders()
{
    while (!bids_.empty())
    {
        auto& bids = bids_.begin()->second;
        if (bids.empty())
        {
            bids_.erase(bids_.begin());
            continue;
        }

        if (bids.front()->GetOrderType() != OrderType::FillAndKill)
        {
            break;
        }

        CancelOrder(bids.front()->GetOrderId());
    }

    while (!asks_.empty())
    {
        auto& asks = asks_.begin()->second;
        if (asks.empty())
        {
            asks_.erase(asks_.begin());
            continue;
        }

        if (asks.front()->GetOrderType() != OrderType::FillAndKill)
        {
            break;
        }

        CancelOrder(asks.front()->GetOrderId());
    }
}

