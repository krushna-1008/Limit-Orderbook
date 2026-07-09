#include "Simulator.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace
{
std::string Trim(std::string value)
{
    auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [&](char c) { return !isSpace(c); }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [&](char c) { return !isSpace(c); }).base(), value.end());
    return value;
}

std::string Upper(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return value;
}

std::vector<std::string> SplitCsv(const std::string& line)
{
    std::vector<std::string> parts;
    std::stringstream stream(line);
    std::string part;

    while (std::getline(stream, part, ','))
    {
        parts.push_back(Upper(Trim(part)));
    }

    return parts;
}

Side ParseSide(const std::string& value)
{
    if (value == "BUY" || value == "B")
    {
        return Side::Buy;
    }
    if (value == "SELL" || value == "S")
    {
        return Side::Sell;
    }
    throw std::invalid_argument("side must be BUY or SELL");
}

OrderType ParseOrderType(const std::string& value)
{
    if (value == "GTC" || value == "GOODTILLCANCEL" || value == "GOOD_TILL_CANCEL")
    {
        return OrderType::GoodTillCancel;
    }
    if (value == "FAK" || value == "FILLANDKILL" || value == "FILL_AND_KILL")
    {
        return OrderType::FillAndKill;
    }
    throw std::invalid_argument("order type must be GTC or FAK");
}

Price ParsePrice(const std::string& value)
{
    return static_cast<Price>(std::stoi(value));
}

Quantity ParseQuantity(const std::string& value)
{
    const auto parsed = std::stoul(value);
    return static_cast<Quantity>(parsed);
}

OrderId ParseOrderId(const std::string& value)
{
    return static_cast<OrderId>(std::stoull(value));
}
}

Simulator::Simulator(std::ostream& output)
    : output_{output}
{
}

bool Simulator::ExecuteFile(const std::string& path)
{
    std::ifstream input(path);
    if (!input)
    {
        output_ << "Unable to open input file: " << path << '\n';
        return false;
    }

    bool ok = true;
    std::string line;
    while (std::getline(input, line))
    {
        ok = ExecuteLine(line) && ok;
    }

    return ok;
}

bool Simulator::ExecuteLine(const std::string& line)
{
    const auto trimmed = Trim(line);
    if (trimmed.empty() || trimmed[0] == '#')
    {
        return true;
    }

    const auto parts = SplitCsv(trimmed);

    try
    {
        Trades trades;
        if (parts.size() == 4)
        {
            auto order = std::make_shared<Order>(
                ParseOrderType(parts[1]),
                nextOrderId_++,
                ParseSide(parts[0]),
                ParsePrice(parts[2]),
                ParseQuantity(parts[3]));
            trades = orderBook_.AddOrder(order);
        }
        else if (parts.size() == 6 && parts[0] == "ADD")
        {
            auto order = std::make_shared<Order>(
                ParseOrderType(parts[3]),
                ParseOrderId(parts[1]),
                ParseSide(parts[2]),
                ParsePrice(parts[4]),
                ParseQuantity(parts[5]));
            nextOrderId_ = std::max(nextOrderId_, order->GetOrderId() + 1);
            trades = orderBook_.AddOrder(order);
        }
        else if (parts.size() == 2 && parts[0] == "CANCEL")
        {
            orderBook_.CancelOrder(ParseOrderId(parts[1]));
        }
        else if (parts.size() == 5 && parts[0] == "MODIFY")
        {
            trades = orderBook_.ModifyOrder(OrderModify{
                ParseOrderId(parts[1]),
                ParseSide(parts[2]),
                ParsePrice(parts[3]),
                ParseQuantity(parts[4])});
        }
        else if (parts.size() == 1 && parts[0] == "PRINT")
        {
            PrintBook();
            return true;
        }
        else
        {
            throw std::invalid_argument("unsupported command");
        }

        PrintTrades(trades);
        return true;
    }
    catch (const std::exception& ex)
    {
        output_ << "Invalid input line: " << line << " (" << ex.what() << ")\n";
        return false;
    }
}

const OrderBook& Simulator::GetOrderBook() const
{
    return orderBook_;
}

void Simulator::PrintTrades(const Trades& trades) const
{
    for (const auto& trade : trades)
    {
        output_ << "TRADE "
                << "bid=" << trade.GetBidTrade().orderId
                << " ask=" << trade.GetAskTrade().orderId
                << " price=" << trade.GetAskTrade().price
                << " quantity=" << trade.GetAskTrade().quantity << '\n';
    }
}

void Simulator::PrintBook() const
{
    const auto levels = orderBook_.GetOrderInfo();

    output_ << "========================\n";
    output_ << "BIDS\n";
    for (const auto& level : levels.GetBids())
    {
        output_ << std::setw(8) << level.price << ' ' << level.quantity << '\n';
    }
    output_ << "------------------------\n";
    output_ << "ASKS\n";
    for (const auto& level : levels.GetAsks())
    {
        output_ << std::setw(8) << level.price << ' ' << level.quantity << '\n';
    }
    output_ << "========================\n";
}

