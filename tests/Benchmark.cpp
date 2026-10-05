#include "../src/OrderBook.h"

#include <chrono>
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>

using namespace std;
using namespace chrono;


// ============================================================
// REALISTIC MIXED ORDER BOOK BENCHMARK
// ============================================================

int main() {

    cout << "========================================\n";
    cout << "     MIXED ORDER BOOK BENCHMARK\n";
    cout << "========================================\n\n";


    // ========================================================
    // CONFIGURATION
    // ========================================================

    const int INITIAL_ORDERS = 100000;

    const int MIXED_OPERATIONS = 100000;

    const int PRICE_LEVELS = 10;


    // ========================================================
    // CREATE ORDER BOOK
    // ========================================================

    OrderBook book;


    // ========================================================
    // RANDOM GENERATOR
    // ========================================================

    mt19937 rng(42);

    uniform_int_distribution<int> quantityDist(
        1,
        100
    );


    // ========================================================
    // PHASE 1
    // BUILD INITIAL ORDER BOOK
    // ========================================================

    cout << "Building initial order book...\n";

    auto initialStart =
        high_resolution_clock::now();


    int nextOrderId = 1;


    // --------------------------------------------------------
    // INITIAL BUY ORDERS
    // --------------------------------------------------------

    for (
        int i = 0;
        i < INITIAL_ORDERS / 2;
        ++i
    ) {

        Order order;

        order.id = nextOrderId++;

        order.side = Side::BUY;

        order.type = OrderType::LIMIT;


        // BUY prices:
        // 99, 98, ..., 90

        order.price =
            99.0 -
            (i % PRICE_LEVELS);


        order.quantity =
            quantityDist(rng);


        order.timestamp =
            order.id;


        book.addOrder(order);
    }


    // --------------------------------------------------------
    // INITIAL SELL ORDERS
    // --------------------------------------------------------

    for (
        int i = 0;
        i < INITIAL_ORDERS / 2;
        ++i
    ) {

        Order order;

        order.id = nextOrderId++;

        order.side = Side::SELL;

        order.type = OrderType::LIMIT;


        // SELL prices:
        // 101, 102, ..., 110

        order.price =
            101.0 +
            (i % PRICE_LEVELS);


        order.quantity =
            quantityDist(rng);


        order.timestamp =
            order.id;


        book.addOrder(order);
    }


    auto initialEnd =
        high_resolution_clock::now();


    long long initialTimeNs =
        duration_cast<nanoseconds>(
            initialEnd - initialStart
        ).count();


    double initialTimeMs =
        initialTimeNs / 1'000'000.0;


    cout << "Initial orders: "
         << INITIAL_ORDERS
         << "\n";


    cout << "Initial construction time: "
         << fixed
         << setprecision(2)
         << initialTimeMs
         << " ms\n\n";


    // ========================================================
    // PHASE 2
    // CANCELLATION BENCHMARK
    // ========================================================

    /*
        Cancel 25,000 known resting BUY orders.

        We select orders from the middle of the book so
        they are guaranteed to exist before cancellation.
    */

    const int CANCEL_COUNT = 25000;


    cout << "Cancelling "
         << CANCEL_COUNT
         << " orders...\n";


    auto cancelStart =
        high_resolution_clock::now();


    int successfulCancels = 0;


    for (
        int i = 0;
        i < CANCEL_COUNT;
        ++i
    ) {

        int orderId =
            1 + i * 2;


        if (
            book.cancelOrder(orderId)
        ) {

            ++successfulCancels;
        }
    }


    auto cancelEnd =
        high_resolution_clock::now();


    long long cancelTimeNs =
        duration_cast<nanoseconds>(
            cancelEnd - cancelStart
        ).count();


    double cancelTimeMs =
        cancelTimeNs / 1'000'000.0;


    double cancelThroughput =
        successfulCancels /
        (cancelTimeNs / 1'000'000'000.0);


    double cancelLatency =
        successfulCancels > 0
        ? static_cast<double>(cancelTimeNs)
          / successfulCancels
        : 0.0;


    // ========================================================
    // PHASE 3
    // MIXED WORKLOAD
    // ========================================================

    cout << "\nRunning "
         << MIXED_OPERATIONS
         << " mixed operations...\n";


    /*
        Workload:

        40% NEW orders
        25% MATCHING orders
        20% MODIFY orders
        15% CANCEL orders

        This gives us a much more realistic workload.
    */


    auto mixedStart =
        high_resolution_clock::now();


    int newOrders = 0;

    int matchingOrders = 0;

    int modifyOperations = 0;

    int cancelOperations = 0;

    int successfulModifications = 0;

    int successfulCancellations = 0;


    // Keep track of IDs that are likely still active.

    vector<int> activeOrderIds;

    activeOrderIds.reserve(
        INITIAL_ORDERS
    );


    // Add a range of existing IDs.

    for (
        int i = CANCEL_COUNT + 1;
        i <= INITIAL_ORDERS;
        ++i
    ) {

        activeOrderIds.push_back(i);
    }


    for (
        int i = 0;
        i < MIXED_OPERATIONS;
        ++i
    ) {

        int operation =
            i % 20;


        // ====================================================
        // NEW ORDER
        // 40%
        // ====================================================

        if (operation < 8) {

            Order order;

            order.id =
                nextOrderId++;


            // Alternate BUY / SELL

            if (i % 2 == 0) {

                order.side =
                    Side::BUY;


                order.type =
                    OrderType::LIMIT;


                /*
                    Crossing BUY.

                    It will match against
                    the best available SELL.
                */

                order.price =
                    101.0 +
                    (i % PRICE_LEVELS);
            }

            else {

                order.side =
                    Side::SELL;


                order.type =
                    OrderType::LIMIT;


                /*
                    Crossing SELL.

                    It will match against
                    the best available BUY.
                */

                order.price =
                    90.0 +
                    (i % PRICE_LEVELS);
            }


            order.quantity =
                quantityDist(rng);


            order.timestamp =
                order.id;


            book.addOrder(order);


            ++newOrders;
        }


        // ====================================================
        // MATCHING ORDER
        // 25%
        // ====================================================

        else if (operation < 13) {

            Order order;

            order.id =
                nextOrderId++;


            if (i % 2 == 0) {

                order.side =
                    Side::BUY;

                order.type =
                    OrderType::LIMIT;

                order.price =
                    101.0 +
                    (i % PRICE_LEVELS);
            }

            else {

                order.side =
                    Side::SELL;

                order.type =
                    OrderType::LIMIT;

                order.price =
                    90.0 +
                    (i % PRICE_LEVELS);
            }


            // Larger quantity increases the
            // probability of partial fills.

            order.quantity =
                50 +
                quantityDist(rng);


            order.timestamp =
                order.id;


            book.addOrder(order);


            ++matchingOrders;
        }


        // ====================================================
        // MODIFY ORDER
        // 20%
        // ====================================================

        else if (operation < 17) {

            if (!activeOrderIds.empty()) {

                int index =
                    i % activeOrderIds.size();


                int orderId =
                    activeOrderIds[index];


                double newPrice;


                if (i % 2 == 0) {

                    newPrice =
                        95.0 +
                        (i % PRICE_LEVELS);
                }

                else {

                    newPrice =
                        105.0 +
                        (i % PRICE_LEVELS);
                }


                int newQuantity =
                    10 +
                    (i % 90);


                ++modifyOperations;


                if (
                    book.modifyOrder(
                        orderId,
                        newPrice,
                        newQuantity
                    )
                ) {

                    ++successfulModifications;
                }
            }
        }


        // ====================================================
        // CANCEL ORDER
        // 15%
        // ====================================================

        else {

            if (!activeOrderIds.empty()) {

                int index =
                    i % activeOrderIds.size();


                int orderId =
                    activeOrderIds[index];


                ++cancelOperations;


                if (
                    book.cancelOrder(orderId)
                ) {

                    ++successfulCancellations;
                }
            }
        }
    }


    auto mixedEnd =
        high_resolution_clock::now();


    // ========================================================
    // MIXED WORKLOAD TIMING
    // ========================================================

    long long mixedTimeNs =
        duration_cast<nanoseconds>(
            mixedEnd - mixedStart
        ).count();


    double mixedTimeMs =
        mixedTimeNs / 1'000'000.0;


    double mixedLatency =
        static_cast<double>(
            mixedTimeNs
        ) / MIXED_OPERATIONS;


    double mixedThroughput =
        MIXED_OPERATIONS /
        (mixedTimeNs / 1'000'000'000.0);


    // ========================================================
    // TOTAL TIME
    // ========================================================

    long long totalTimeNs =
        initialTimeNs +
        cancelTimeNs +
        mixedTimeNs;


    double totalTimeMs =
        totalTimeNs / 1'000'000.0;


    // ========================================================
    // TOTAL OPERATIONS
    // ========================================================

    int totalOperations =
        INITIAL_ORDERS +
        successfulCancels +
        MIXED_OPERATIONS;


    double overallThroughput =
        totalOperations /
        (totalTimeNs / 1'000'000'000.0);


    // ========================================================
    // TRADE COUNT
    // ========================================================

    int totalTrades =
        book.getTradeCount();


    // ========================================================
    // RESULTS
    // ========================================================

    cout << "\n";

    cout << "========================================\n";

    cout << "          BENCHMARK RESULTS\n";

    cout << "========================================\n";


    cout << fixed
         << setprecision(2);


    // --------------------------------------------------------
    // ORDER COUNTS
    // --------------------------------------------------------

    cout << "\n";

    cout << "Initial orders             : "
         << INITIAL_ORDERS
         << "\n";


    cout << "Successful cancellations   : "
         << successfulCancels
         << "\n";


    cout << "Mixed operations           : "
         << MIXED_OPERATIONS
         << "\n";


    cout << "Total trades executed      : "
         << totalTrades
         << "\n";


    // --------------------------------------------------------
    // MIXED WORKLOAD BREAKDOWN
    // --------------------------------------------------------

    cout << "\n";

    cout << "------ MIXED WORKLOAD ------\n";


    cout << "New orders                 : "
         << newOrders
         << "\n";


    cout << "Matching orders            : "
         << matchingOrders
         << "\n";


    cout << "Modify attempts            : "
         << modifyOperations
         << "\n";


    cout << "Successful modifications   : "
         << successfulModifications
         << "\n";


    cout << "Cancel attempts            : "
         << cancelOperations
         << "\n";


    cout << "Successful cancellations   : "
         << successfulCancellations
         << "\n";


    // --------------------------------------------------------
    // CANCELLATION PERFORMANCE
    // --------------------------------------------------------

    cout << "\n";

    cout << "------ CANCELLATION ------\n";


    cout << "Cancellation time          : "
         << cancelTimeMs
         << " ms\n";


    cout << "Average cancel latency     : "
         << cancelLatency
         << " ns\n";


    cout << "Cancellation throughput    : "
         << cancelThroughput
         << " orders/sec\n";


    // --------------------------------------------------------
    // MIXED PERFORMANCE
    // --------------------------------------------------------

    cout << "\n";

    cout << "------ MIXED WORKLOAD ------\n";


    cout << "Mixed execution time       : "
         << mixedTimeMs
         << " ms\n";


    cout << "Average operation latency  : "
         << mixedLatency
         << " ns\n";


    cout << "Mixed throughput           : "
         << mixedThroughput
         << " operations/sec\n";


    // --------------------------------------------------------
    // TOTAL PERFORMANCE
    // --------------------------------------------------------

    cout << "\n";

    cout << "------ OVERALL ------\n";


    cout << "Total execution time       : "
         << totalTimeMs
         << " ms\n";


    cout << "Overall throughput         : "
         << overallThroughput
         << " operations/sec\n";


    cout << "\n========================================\n";


    return 0;
}