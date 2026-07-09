# Architecture

The project is organized around a small matching-engine core plus a CLI simulator.

## Core Types

- `Order` stores immutable order attributes and remaining quantity.
- `OrderModify` represents a cancel-replace update.
- `Trade` records the bid-side and ask-side execution details.
- `OrderBook` owns the bid book, ask book, and order-id index.
- `Simulator` parses CSV commands and prints trades and book depth.

## Matching Algorithm

The book uses price-time priority:

- Bids are stored in descending price order.
- Asks are stored in ascending price order.
- Orders at the same price level are stored in insertion order.
- The best bid and best ask match while the bid price is greater than or equal to the ask price.

Each match fills the smaller remaining quantity between the oldest bid and oldest ask at the top of book. Filled orders are removed from the price level and the order-id index. Partially filled orders remain at their current position.

## Complexity

- Add order: `O(log P)` to locate the price level, where `P` is the number of price levels, plus any matching work.
- Cancel order: `O(log P)` to erase the price level when it becomes empty and `O(1)` to locate the order by id.
- Modify order: cancel plus add.
- Market depth aggregation: `O(N)` over resting orders.

## CSV Simulator

The CLI accepts the compact roadmap format:

```text
BUY,GTC,100,50
SELL,GTC,99,20
BUY,FAK,101,100
PRINT
```

It also accepts explicit ids and lifecycle commands:

```text
ADD,1,BUY,GTC,100,50
MODIFY,1,BUY,101,25
CANCEL,1
PRINT
```

