# High-Performance C++ Limit Order Matching Engine

A low-latency limit order matching engine implemented in modern C++17, designed to simulate the core functionality of an electronic exchange order book.

The engine supports price-time priority matching, market and limit orders, order cancellation, modification, multiple Time-in-Force policies, trade history, execution reports, automated testing, and performance benchmarking.

---

## Features

- Price-Time Priority matching
- BUY and SELL limit orders
- Market orders
- Partial and full order matching
- Order cancellation
- Order modification
- Order lifecycle/status tracking
- IOC (Immediate-or-Cancel)
- FOK (Fill-or-Kill)
- POST_ONLY orders
- Trade history
- Execution reports
- Fast order lookup using hash indexing
- Price-level organization using ordered maps
- Node-based order storage
- Automated test suite
- Performance benchmarking

---

## Architecture

```text
                    Client / Trader
                          |
                          v
                 +------------------+
                 | MatchingEngine   |
                 +--------+---------+
                          |
                          v
                 +------------------+
                 |    OrderBook     |
                 +--------+---------+
                          |
              +-----------+-----------+
              |                       |
              v                       v
        +-----------+           +-----------+
        |   Bids    |           |   Asks    |
        | BUY Orders|           |SELL Orders|
        +-----+-----+           +-----+-----+
              |                       |
              +-----------+-----------+
                          |
                          v
                  Matching Algorithm
                          |
                          v
                   Trade Execution
                          |
                          v
                 Execution Reports

Project Structure
LimitOrderBook/
│
├── include/
│   ├── Order.h
│   ├── ExecutionReport.h
│   └── MatchingEngine.h
│
├── src/
│   ├── Order.cpp
│   ├── OrderBook.cpp
│   ├── OrderBook.h
│   ├── MatchingEngine.cpp
│   └── main.cpp
│
├── tests/
│   └── TestOrderBook.cpp
│
├── README.md
└── .gitignore

Core Concepts
Price-Time Priority
The matching engine follows the standard price-time priority principle.
For BUY orders:
Higher price = higher priority

Example:
BUY 100 @ $105
BUY 100 @ $103
BUY 100 @ $100

The $105 order is matched first.
For SELL orders:
Lower price = higher priority

Example:
SELL 100 @ $100
SELL 100 @ $103
SELL 100 @ $105

The $100 order is matched first.
If two orders have the same price, the order submitted earlier receives priority.
Order Types
Limit Order
A limit order specifies the maximum price a buyer is willing to pay or the minimum price a seller is willing to accept.
Example:
BUY  100 shares @ $100
SELL 50 shares @ $101

Orders execute only when compatible prices are available.
Market Order
A market order executes against the best available prices in the order book.
Example:
BUY 100 shares @ MARKET

The order consumes available sell liquidity starting from the best ask.
Order Lifecycle
Orders can move through the following states:
                    +------------------+
                    |      OPEN        |
                    +--------+---------+
                             |
              +--------------+--------------+
              |              |              |
              v              v              v
       PARTIALLY_FILLED   CANCELLED      FILLED
              |
              v
           FILLED

Rejected orders are also tracked using the REJECTED status.
Supported statuses include:
OPEN
PARTIALLY_FILLED
FILLED
CANCELLED
REJECTED
UNKNOWN

Time In Force
The engine supports multiple order execution policies.
GTC — Good Till Cancelled
The order remains active until it is completely filled or explicitly cancelled.
IOC — Immediate Or Cancel
The executable portion of the order is matched immediately.
Any remaining quantity is cancelled.
Example:
Requested quantity : 100
Available quantity : 60

Executed           : 60
Cancelled          : 40

FOK — Fill Or Kill
The entire order must be filled immediately.
If sufficient liquidity is unavailable, the order does not execute partially.
Example:
Requested quantity : 100
Available quantity : 80

Result             : Order not executed

POST_ONLY
A POST_ONLY order is prevented from immediately trading against existing liquidity.
If submitting the order would result in an immediate trade, the order is rejected.
Otherwise, it is added to the order book as liquidity.
This is useful for ensuring that an order only provides liquidity rather than taking existing liquidity.
Order Cancellation
Orders can be cancelled using their unique order ID.
The engine maintains an order ID index for efficient lookup.
Conceptually:
Order ID
   |
   v
unordered_map
   |
   v
Order Node
   |
   v
Price Level

When an order is cancelled:
1. It is removed from its price level.
2. It is removed from the active order index.
3. Its status becomes CANCELLED.
4. Its remaining active quantity becomes zero.
Order Modification
Existing orders can be modified by changing:
- Price
- Quantity
When an order is modified, the previous order is removed and the updated order is re-entered.
This means the modified order loses its previous time priority.
This behavior prevents an order from changing its price or quantity while unfairly retaining its original position in the queue.
Order Book Structure
The order book maintains two primary sides:
                 ORDER BOOK
                     |
          +----------+----------+
          |                     |
          v                     v
        BIDS                   ASKS
       BUY SIDE              SELL SIDE

Bids
Bids are maintained in descending price order:
105
103
101
100

The highest bid is therefore immediately accessible.
Asks
Asks are maintained in ascending price order:
100
101
103
105

The lowest ask is therefore immediately accessible.
Data Structures
The implementation uses several C++ standard library data structures.
std::map
Used to organize price levels.
std::map<double, PriceLevel>

This maintains prices in sorted order and allows efficient access to the best price.
std::unordered_map
Used for order ID lookup.
std::unordered_map<int, NodeIndex>

This allows an order to be located directly using its ID.
std::vector
Used as contiguous storage for order nodes.
The implementation also maintains a free-node list so released node slots can be reused.
Node-Based Order Storage
Each active order is represented by an internal node containing:
+----------------------+
| Order                |
+----------------------+
| Previous Node        |
+----------------------+
| Next Node            |
+----------------------+
| Active State         |
+----------------------+

Orders at the same price level are linked together.
This allows the engine to preserve FIFO ordering within a price level.
Matching Process
When a new order arrives, the engine checks whether it can match against the opposite side of the book.
For a BUY order:
Incoming BUY
     |
     v
Best ASK
     |
     v
Is Ask Price <= Buy Price?
     |
   +---+
   |   |
  YES  NO
   |   |
   v   v
 MATCH STOP

For a SELL order:
Incoming SELL
     |
     v
Best BID
     |
     v
Is Bid Price >= Sell Price?
     |
   +---+
   |   |
  YES  NO
   |   |
   v   v
 MATCH STOP

The engine continues matching until:
- The incoming order is completely filled.
- No compatible order remains.
- The order's execution policy prevents further execution.
Trade Execution
When compatible BUY and SELL orders are found, a trade is generated.
A trade contains:
- Buy order ID
- Sell order ID
- Trade price
- Trade quantity
Example:
BUY Order  : 2
SELL Order : 1

Trade Price    : $100
Trade Quantity : 30

All executed trades are stored in trade history.
Trade History
The engine maintains a history of executed trades.
This allows previously executed trades to be inspected after matching.
Example:
TRADE EXECUTED

Buy Order ID  : 2
Sell Order ID : 1
Trade Price   : 100
Trade Quantity: 30

Execution Reports
The MatchingEngine provides an execution-report layer above the underlying order book.
Supported execution types include:
ACCEPTED
FILLED
PARTIALLY_FILLED
CANCELLED
REJECTED
MODIFIED

Example:
Order ID : 2
Type     : FILLED
Quantity : 30
Price    : 100

This separates the core matching logic from the external representation of order events.
MatchingEngine Layer
The project uses a MatchingEngine abstraction on top of the OrderBook.
             MatchingEngine
                   |
        +----------+----------+
        |          |          |
        v          v          v
      Submit     Cancel     Modify
        |
        v
     OrderBook
        |
        v
    Trade Engine
        |
        v
Execution Reports

This provides a cleaner interface for submitting and managing orders without directly exposing all internal order-book operations.
Testing
The project includes an automated test suite covering core functionality.
Test Coverage
1.  Basic BUY order
2.  Basic SELL order
3.  Full match
4.  Partial match
5.  Price priority
6.  Time priority
7.  Cancel order
8.  Invalid cancellation
9.  Modify order
10. Market order
11. IOC
12. FOK success
13. FOK failure
14. POST_ONLY
15. Trade history
16. MatchingEngine
17. MatchingEngine cancel
18. MatchingEngine modify
19. Execution reports
20. Non-crossing orders

Current Test Result
Passed : 59
Failed : 0
Total  : 59

ALL TESTS PASSED!

The test suite verifies not only successful execution but also order lifecycle behavior, cancellation, modification, execution policies, priority rules, and execution reporting.
Performance Benchmark
The engine was benchmarked using a workload containing large numbers of orders and mixed order-book operations.
Initial Workload
Initial orders             : 100000
Successful cancellations   : 25000
Mixed operations           : 100000
Total trades executed      : 109433

Cancellation Performance
Cancellation time          : 3.01 ms
Average cancel latency     : 120.29 ns
Cancellation throughput    : 8313104.78 orders/sec

Approximately:
8.31 million cancellations/sec

Mixed Workload
The mixed workload consisted of:
New orders                 : 40000
Matching orders            : 25000
Modify attempts            : 20000
Successful modifications   : 11469
Cancel attempts            : 15000
Successful cancellations   : 10000

Results:
Mixed execution time       : 39.03 ms
Average operation latency  : 390.27 ns
Mixed throughput           : 2562341.78 operations/sec

Approximately:
2.56 million operations/sec

Overall Performance
Total execution time       : 74.70 ms
Overall throughput         : 3011870.79 operations/sec

Approximately:
3.01 million operations/sec

Performance results depend on hardware, compiler configuration, workload distribution, and runtime environment. These figures represent measurements from the implemented benchmark and should not be interpreted as exchange-grade production performance.

Build Instructions
The project requires a C++17-compatible compiler.
Compile the Main Program
From the project root:
g++ -std=c++17 src/*.cpp -Iinclude -o matching_engine

Run:
./matching_engine

Windows PowerShell
g++ -std=c++17 src/*.cpp -Iinclude -o matching_engine

Then:
./matching_engine

Running Tests
The test executable should be compiled without src/main.cpp to avoid multiple main() definitions.
g++ -std=c++17 src/Order.cpp src/OrderBook.cpp src/MatchingEngine.cpp tests/TestOrderBook.cpp -Iinclude -Isrc -o test_engine

Run:
./test_engine

Expected output:
Passed : 59
Failed : 0
Total  : 59

ALL TESTS PASSED!

Design Goals
The project focuses on several important properties of electronic trading systems.
Correctness
The engine must correctly enforce:
- Price priority
- Time priority
- Matching conditions
- Order quantities
- Order states
- Time-in-Force rules
Performance
The implementation uses:
- Ordered price-level maps
- Hash-based order lookup
- Reusable node storage
- Linked order queues
- Avoidance of unnecessary data movement
Determinism
Given the same sequence of orders, the matching engine should produce the same sequence of executions.
Maintainability
The project separates:
Order
   |
OrderBook
   |
MatchingEngine
   |
Execution Reports

This keeps responsibilities separated and makes the system easier to extend.
Technologies
Language        : C++17
Compiler        : g++
Data Structures : std::map
                  std::unordered_map
                  std::vector
Testing         : Custom C++ test suite
Build           : g++
Version Control : Git

Example Workflow
A typical order flow looks like:
1. Trader submits order
            |
            v
2. MatchingEngine receives order
            |
            v
3. Order validation
            |
            v
4. OrderBook receives order
            |
            v
5. Check opposite-side liquidity
            |
            v
6. Apply price-time priority
            |
            v
7. Execute trade(s)
            |
            v
8. Update quantities/status
            |
            v
9. Store trade history
            |
            v
10. Generate execution report

Example
Suppose the order book contains:
BIDS

Price    Quantity
-----------------
99       100
98       200
97       150

And:
ASKS

Price    Quantity
-----------------
101      100
102      200
103      150

A new order arrives:
BUY 150 @ 101

The engine sees:
Best Ask = 101
Buy Price = 101

Since:
101 <= 101

the order can execute.
The result is:
BUY quantity : 150
SELL quantity: 100 @ 101

Remaining BUY quantity:
50

The engine then continues matching against the next compatible ask according to the implemented matching rules.
Project Status
Core Matching Engine       : Complete
Order Management            : Complete
Cancellation               : Complete
Modification               : Complete
Market Orders              : Complete
IOC                         : Complete
FOK                         : Complete
POST_ONLY                   : Complete
Trade History              : Complete
Execution Reports          : Complete
Automated Tests             : Complete
Performance Benchmark      : Complete
Documentation              : Complete

Current test status:
59 / 59 tests passing

Future Improvements
Potential future extensions include:
- FIX protocol integration
- Network-based order submission
- Market-data feed integration
- Persistent event logging
- Market replay
- More detailed latency instrumentation
- Memory-pool optimization
- Lock-free data structures
- Multithreaded market-data processing
- More extensive benchmark workloads
- Production-grade risk controls
These features are intentionally outside the current core implementation.
Disclaimer
This project is an educational and software-engineering implementation of a limit order matching engine.
It is not connected to a live financial exchange and should not be used for real trading without substantial additional engineering, testing, security, risk-management, and regulatory controls.
Author
Maltoh Sravani
Integrated Msc  — mathematics
NIT Rourkela 
                