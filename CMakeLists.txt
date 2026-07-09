cmake_minimum_required(VERSION 3.16)

project(OrderBook VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(orderbook_core
    src/Order.cpp
    src/OrderModify.cpp
    src/OrderBook.cpp
    src/Simulator.cpp
    src/Trade.cpp
)

target_include_directories(orderbook_core PUBLIC include)

add_executable(orderbook src/main.cpp)
target_link_libraries(orderbook PRIVATE orderbook_core)

enable_testing()

add_executable(orderbook_tests tests/OrderBookTests.cpp)
target_link_libraries(orderbook_tests PRIVATE orderbook_core)
add_test(NAME orderbook_tests COMMAND orderbook_tests)

add_executable(orderbook_benchmark benchmarks/benchmark.cpp)
target_link_libraries(orderbook_benchmark PRIVATE orderbook_core)

