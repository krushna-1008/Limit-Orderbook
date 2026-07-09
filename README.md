# Electronic Limit Order Book

A modern C++17 electronic limit order book with price-time priority matching, order lifecycle operations, CSV simulation, unit tests, and a simple performance benchmark.

## Features

- Good-Till-Cancel (GTC) orders
- Fill-and-Kill (FAK) orders
- Price-time priority matching
- Partial and complete fills
- Order add, cancel, and modify operations
- Trade generation
- Aggregated market depth
- CSV-driven simulation

## Project Structure

```text
include/      Public headers
src/          Matching engine and CLI implementation
tests/        Self-contained unit tests
benchmarks/   Lightweight throughput benchmark
examples/     Sample order input files
docs/         Architecture notes and diagrams
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

If CMake is not installed, you can compile directly:

```bash
g++ -std=c++17 -Iinclude src/*.cpp -o orderbook
```

## Run

```bash
./build/orderbook examples/orders.csv
```

Input supports the compact format:

```text
BUY,GTC,100,50
SELL,GTC,99,20
BUY,FAK,101,100
PRINT
```

It also supports explicit lifecycle commands:

```text
ADD,1,BUY,GTC,100,50
MODIFY,1,BUY,101,25
CANCEL,1
PRINT
```

Example book output:

```text
========================
BIDS
     101 30
     100 50
------------------------
ASKS
     103 25
========================
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

Without CMake:

```bash
g++ -std=c++17 -Iinclude tests/OrderBookTests.cpp src/Order.cpp src/OrderModify.cpp src/OrderBook.cpp src/Simulator.cpp src/Trade.cpp -o orderbook_tests
./orderbook_tests
```

## Benchmark

```bash
./build/orderbook_benchmark 100000
```

The benchmark reports total orders, generated trades, resting orders, elapsed seconds, and orders per second. Use larger inputs such as `500000`, `1000000`, or `5000000` for heavier workloads.

## Algorithm

The book stores bids in descending price order and asks in ascending price order. Each price level stores orders in insertion order, so matching naturally follows FIFO priority within a level. Matching continues while the best bid is greater than or equal to the best ask.

See [docs/architecture.md](docs/architecture.md) for more detail.

## Future Improvements

- Immediate-Or-Cancel (IOC) orders
- Market orders
- Iceberg orders
- Stop orders
- Optional thread-safe wrapper
- GoogleTest or Catch2 integration for richer test reporting
