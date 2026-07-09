#include "Simulator.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    Simulator simulator{std::cout};

    if (argc > 2)
    {
        std::cerr << "Usage: orderbook [orders.csv]\n";
        return 1;
    }

    if (argc == 2)
    {
        return simulator.ExecuteFile(argv[1]) ? 0 : 1;
    }

    std::cout << "No input file provided; running sample simulation.\n";
    simulator.ExecuteLine("BUY,GTC,100,50");
    simulator.ExecuteLine("SELL,GTC,99,20");
    simulator.ExecuteLine("BUY,FAK,101,100");
    simulator.ExecuteLine("PRINT");
    return 0;
}

