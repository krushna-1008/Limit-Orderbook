#pragma once

#include "Order.hpp"

#include <memory>

using OrderPointer = std::shared_ptr<Order>;

class OrderModify
{
public:
    OrderModify(OrderId orderId, Side side, Price price, Quantity quantity);

    OrderId GetOrderId() const;
    Price GetPrice() const;
    Side GetSide() const;
    Quantity GetQuantity() const;

    OrderPointer ToOrderPointer(OrderType type) const;

private:
    OrderId orderId_;
    Side side_;
    Price price_;
    Quantity quantity_;
};

