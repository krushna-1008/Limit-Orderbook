#pragma once

#include "Types.hpp"

class Order
{
public:
    Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity);

    OrderId GetOrderId() const;
    Side GetSide() const;
    Price GetPrice() const;
    OrderType GetOrderType() const;
    Quantity GetInitialQuantity() const;
    Quantity GetRemainingQuantity() const;
    Quantity GetFilledQuantity() const;
    bool IsFilled() const;

    void Fill(Quantity quantity);

private:
    OrderType orderType_;
    OrderId orderId_;
    Side side_;
    Price price_;
    Quantity initialQuantity_;
    Quantity remainingQuantity_;
};

