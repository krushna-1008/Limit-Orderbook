#include "OrderBook.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

int main(int argc, char* argv[])
{
    const std::size_t orderCount = argc > 1 ? static_cast<std::size_t>(std::stoull(argv[1])) : 100000;

    std::mt19937 rng{42};
    std::uniform_int_distribution<int> sideDist{0, 1};
    std::uniform_int_distribution<int> priceDist{90, 110};
    std::uniform_int_distribution<int> quantityDist{1, 100};

    std::vector<OrderPointer> orders;
    orders.reserve(orderCount);

    for (std::size_t i = 0; i < orderCount; ++i)
    {
        orders.push_back(std::make_shared<Order>(
            OrderType::GoodTillCancel,
            static_cast<OrderId>(i + 1),
            sideDist(rng) == 0 ? Side::Buy : Side::Sell,
            static_cast<Price>(priceDist(rng)),
            static_cast<Quantity>(quantityDist(rng))));
    }

    OrderBook book;
    const auto started = std::chrono::steady_clock::now();
    std::size_t tradeCount = 0;

    for (const auto& order : orders)
    {
        tradeCount += book.AddOrder(order).size();
    }

    const auto finished = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration<double>(finished - started).count();

    std::cout << "orders=" << orderCount << '\n';
    std::cout << "trades=" << tradeCount << '\n';
    std::cout << "resting_orders=" << book.Size() << '\n';
    std::cout << "seconds=" << elapsed << '\n';
    std::cout << "orders_per_second=" << static_cast<double>(orderCount) / elapsed << '\n';

    return 0;
}

