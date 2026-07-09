#include "OrderBook.hpp"
#include "Simulator.hpp"

#include <cassert>
#include <memory>
#include <sstream>

namespace
{
OrderPointer MakeOrder(OrderType type, OrderId id, Side side, Price price, Quantity quantity)
{
    return std::make_shared<Order>(type, id, side, price, quantity);
}

void TestAddAndCancelOrder()
{
    OrderBook book;

    book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10));
    assert(book.Size() == 1);

    book.CancelOrder(1);
    assert(book.Size() == 0);
}

void TestModifyOrder()
{
    OrderBook book;

    book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10));
    book.ModifyOrder(OrderModify{1, Side::Buy, 101, 25});

    const auto levels = book.GetOrderInfo();
    assert(levels.GetBids().size() == 1);
    assert(levels.GetBids()[0].price == 101);
    assert(levels.GetBids()[0].quantity == 25);
}

void TestPartialFill()
{
    OrderBook book;

    book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10));
    const auto trades = book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 2, Side::Sell, 100, 4));

    assert(trades.size() == 1);
    assert(trades[0].GetBidTrade().quantity == 4);
    assert(book.Size() == 1);
    assert(book.GetOrderInfo().GetBids()[0].quantity == 6);
}

void TestCompleteFill()
{
    OrderBook book;

    book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10));
    const auto trades = book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 2, Side::Sell, 99, 10));

    assert(trades.size() == 1);
    assert(book.Size() == 0);
}

void TestFifoMatching()
{
    OrderBook book;

    book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 1, Side::Buy, 100, 5));
    book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 2, Side::Buy, 100, 5));
    const auto trades = book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 3, Side::Sell, 100, 7));

    assert(trades.size() == 2);
    assert(trades[0].GetBidTrade().orderId == 1);
    assert(trades[0].GetBidTrade().quantity == 5);
    assert(trades[1].GetBidTrade().orderId == 2);
    assert(trades[1].GetBidTrade().quantity == 2);
}

void TestFillAndKill()
{
    OrderBook book;

    book.AddOrder(MakeOrder(OrderType::FillAndKill, 1, Side::Buy, 100, 10));
    assert(book.Size() == 0);

    book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 2, Side::Sell, 99, 4));
    const auto trades = book.AddOrder(MakeOrder(OrderType::FillAndKill, 3, Side::Buy, 100, 10));

    assert(trades.size() == 1);
    assert(trades[0].GetBidTrade().quantity == 4);
    assert(book.Size() == 0);
}

void TestMultiplePriceLevels()
{
    OrderBook book;

    book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10));
    book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 2, Side::Buy, 101, 5));
    book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 3, Side::Sell, 105, 8));
    book.AddOrder(MakeOrder(OrderType::GoodTillCancel, 4, Side::Sell, 104, 2));

    const auto levels = book.GetOrderInfo();
    assert(levels.GetBids().size() == 2);
    assert(levels.GetAsks().size() == 2);
    assert(levels.GetBids()[0].price == 101);
    assert(levels.GetAsks()[0].price == 104);
}

void TestCsvSimulator()
{
    std::ostringstream output;
    Simulator simulator{output};

    assert(simulator.ExecuteLine("ADD,10,BUY,GTC,100,10"));
    assert(simulator.ExecuteLine("SELL,GTC,99,3"));
    assert(simulator.ExecuteLine("PRINT"));
    assert(output.str().find("TRADE") != std::string::npos);
    assert(output.str().find("BIDS") != std::string::npos);
}
}

int main()
{
    TestAddAndCancelOrder();
    TestModifyOrder();
    TestPartialFill();
    TestCompleteFill();
    TestFifoMatching();
    TestFillAndKill();
    TestMultiplePriceLevels();
    TestCsvSimulator();
    return 0;
}

