#pragma once

#include "OrderBook.hpp"

#include <iosfwd>
#include <string>

class Simulator
{
public:
    explicit Simulator(std::ostream& output);

    bool ExecuteFile(const std::string& path);
    bool ExecuteLine(const std::string& line);
    const OrderBook& GetOrderBook() const;

private:
    void PrintTrades(const Trades& trades) const;
    void PrintBook() const;

    OrderBook orderBook_;
    std::ostream& output_;
    OrderId nextOrderId_{1};
};

