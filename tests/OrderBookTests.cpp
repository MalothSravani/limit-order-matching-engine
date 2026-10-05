#include <iostream>
#include <string>

#include "../src/OrderBook.h"

using namespace std;


// ============================================================
// TEST COUNTERS
// ============================================================

int passed = 0;
int failed = 0;


// ============================================================
// TEST HELPER
// ============================================================

void check(bool condition, const string& testName) {

    if (condition) {

        cout << "[PASS] " << testName << "\n";
        ++passed;

    } else {

        cout << "[FAIL] " << testName << "\n";
        ++failed;
    }
}


// ============================================================
// TEST 1: BASIC MATCHING
// ============================================================

void testBasicMatching() {

    OrderBook book;

    book.addOrder({
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        1
    });

    book.addOrder({
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        2
    });

    check(
        book.getTradeCount() == 1,
        "Basic matching creates one trade"
    );

    check(
        book.getBestAsk() == -1,
        "Filled SELL order is removed"
    );

    check(
        book.getBestBid() == -1,
        "Filled BUY order is not stored"
    );
}


// ============================================================
// TEST 2: PARTIAL FILL
// ============================================================

void testPartialFill() {

    OrderBook book;

    book.addOrder({
        1,
        Side::SELL,
        OrderType::LIMIT,
        101.0,
        100,
        1
    });

    book.addOrder({
        2,
        Side::BUY,
        OrderType::LIMIT,
        101.0,
        40,
        2
    });

    check(
        book.getTradeCount() == 1,
        "Partial fill creates one trade"
    );

    check(
        book.getOrderQuantity(1) == 60,
        "Remaining SELL quantity is 60"
    );
}


// ============================================================
// TEST 3: PRICE PRIORITY
// ============================================================

void testPricePriority() {

    OrderBook book;

    book.addOrder({
        1,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        1
    });

    book.addOrder({
        2,
        Side::BUY,
        OrderType::LIMIT,
        101.0,
        50,
        2
    });

    book.addOrder({
        3,
        Side::SELL,
        OrderType::LIMIT,
        99.0,
        60,
        3
    });

    check(
        book.getTradeCount() == 2,
        "Price priority creates two trades"
    );

    check(
        book.getOrderQuantity(2) == -1,
        "Higher-priced BUY gets priority"
    );

    check(
        book.getOrderQuantity(1) == 40,
        "Lower-priced BUY receives remaining quantity"
    );
}


// ============================================================
// TEST 4: TIME PRIORITY
// ============================================================

void testTimePriority() {

    OrderBook book;

    book.addOrder({
        1,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        10
    });

    book.addOrder({
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        20
    });

    book.addOrder({
        3,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        60,
        30
    });

    check(
        book.getOrderQuantity(1) == -1,
        "Earlier BUY gets time priority"
    );

    check(
        book.getOrderQuantity(2) == 40,
        "Later BUY receives remaining quantity"
    );
}


// ============================================================
// TEST 5: MARKET BUY
// ============================================================

void testMarketBuy() {

    OrderBook book;

    book.addOrder({
        1,
        Side::SELL,
        OrderType::LIMIT,
        101.0,
        50,
        1
    });

    book.addOrder({
        2,
        Side::SELL,
        OrderType::LIMIT,
        102.0,
        50,
        2
    });

    book.addOrder({
        3,
        Side::BUY,
        OrderType::MARKET,
        0,
        80,
        3
    });

    check(
        book.getTradeCount() == 2,
        "Market BUY consumes multiple price levels"
    );

    check(
        book.getOrderQuantity(1) == -1,
        "First SELL is completely filled"
    );

    check(
        book.getOrderQuantity(2) == 20,
        "Second SELL is partially filled"
    );

    check(
        book.getOrderQuantity(3) == -1,
        "Market BUY is not stored"
    );
}


// ============================================================
// TEST 6: MARKET SELL
// ============================================================

void testMarketSell() {

    OrderBook book;

    book.addOrder({
        1,
        Side::BUY,
        OrderType::LIMIT,
        101.0,
        50,
        1
    });

    book.addOrder({
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        2
    });

    book.addOrder({
        3,
        Side::SELL,
        OrderType::MARKET,
        0,
        80,
        3
    });

    check(
        book.getTradeCount() == 2,
        "Market SELL consumes multiple price levels"
    );

    check(
        book.getOrderQuantity(1) == -1,
        "Best BUY is completely filled"
    );

    check(
        book.getOrderQuantity(2) == 20,
        "Second BUY is partially filled"
    );
}


// ============================================================
// TEST 7: CANCELLATION
// ============================================================

void testCancellation() {

    OrderBook book;

    book.addOrder({
        1,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        1
    });

    check(
        book.cancelOrder(1),
        "Existing order can be cancelled"
    );

    check(
        book.getOrderQuantity(1) == -1,
        "Cancelled order is removed"
    );

    check(
        !book.cancelOrder(1),
        "Already cancelled order cannot be cancelled again"
    );
}


// ============================================================
// TEST 8: MODIFICATION
// ============================================================

void testModification() {

    OrderBook book;

    book.addOrder({
        1,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        10
    });

    check(
        book.modifyOrder(1, 99.0, 80),
        "Order modification succeeds"
    );

    check(
        book.getBestBid() == 99.0,
        "Modified price is updated"
    );

    check(
        book.getOrderQuantity(1) == 80,
        "Modified quantity is updated"
    );
}


// ============================================================
// TEST 9: MODIFICATION LOSES TIME PRIORITY
// ============================================================

void testModificationPriority() {

    OrderBook book;

    book.addOrder({
        1,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        10
    });

    book.addOrder({
        2,
        Side::BUY,
        OrderType::LIMIT,
        99.0,
        50,
        20
    });

    book.modifyOrder(
        1,
        99.0,
        80
    );

    book.addOrder({
        3,
        Side::SELL,
        OrderType::LIMIT,
        99.0,
        60,
        30
    });

    check(
        book.getOrderQuantity(2) == -1,
        "Older order keeps time priority after modification"
    );

    check(
        book.getOrderQuantity(1) == 70,
        "Modified order receives remaining quantity"
    );
}


// ============================================================
// TEST 10: INVALID QUANTITY
// ============================================================

void testInvalidQuantity() {

    OrderBook book;

    check(
        !book.addOrder({
            1,
            Side::BUY,
            OrderType::LIMIT,
            100.0,
            -10,
            1
        }),
        "Negative quantity is rejected"
    );
}


// ============================================================
// TEST 11: INVALID PRICE
// ============================================================

void testInvalidPrice() {

    OrderBook book;

    check(
        !book.addOrder({
            1,
            Side::BUY,
            OrderType::LIMIT,
            -100.0,
            10,
            1
        }),
        "Negative LIMIT price is rejected"
    );
}


// ============================================================
// TEST 12: DUPLICATE ORDER ID
// ============================================================

void testDuplicateID() {

    OrderBook book;

    check(
        book.addOrder({
            1,
            Side::BUY,
            OrderType::LIMIT,
            100.0,
            10,
            1
        }),
        "First order is accepted"
    );

    check(
        !book.addOrder({
            1,
            Side::SELL,
            OrderType::LIMIT,
            101.0,
            10,
            2
        }),
        "Duplicate active order ID is rejected"
    );
}


// ============================================================
// TEST 13: INVALID CANCELLATION
// ============================================================

void testInvalidCancellation() {

    OrderBook book;

    check(
        !book.cancelOrder(999),
        "Cancellation of missing order is rejected"
    );
}


// ============================================================
// TEST 14: INVALID MODIFICATION
// ============================================================

void testInvalidModification() {

    OrderBook book;

    check(
        !book.modifyOrder(999, 100.0, 50),
        "Modification of missing order is rejected"
    );
}


// ============================================================
// TEST 15: ORDER STATUS
// ============================================================

void testOrderStatuses() {

    OrderBook book;


    // --------------------------------------------------------
    // OPEN
    // --------------------------------------------------------

    book.addOrder({
        1,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        100,
        1
    });

    check(
        book.getOrderStatus(1) == OrderStatus::OPEN,
        "New resting order gets OPEN status"
    );


    // --------------------------------------------------------
    // PARTIAL FILL
    // --------------------------------------------------------

    book.addOrder({
        2,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        40,
        2
    });

    check(
        book.getOrderStatus(1)
        == OrderStatus::PARTIALLY_FILLED,
        "Partially filled order gets PARTIALLY_FILLED status"
    );


    // --------------------------------------------------------
    // FILLED
    // --------------------------------------------------------

    check(
        book.getOrderStatus(2)
        == OrderStatus::FILLED,
        "Completely filled incoming order gets FILLED status"
    );


    // --------------------------------------------------------
    // CANCELLED
    // --------------------------------------------------------

    book.addOrder({
        3,
        Side::BUY,
        OrderType::LIMIT,
        90.0,
        50,
        3
    });

    book.cancelOrder(3);

    check(
        book.getOrderStatus(3)
        == OrderStatus::CANCELLED,
        "Cancelled order gets CANCELLED status"
    );


    // --------------------------------------------------------
    // REJECTED
    // --------------------------------------------------------

    book.addOrder({
        4,
        Side::BUY,
        OrderType::LIMIT,
        -10.0,
        50,
        4
    });

    check(
        book.getOrderStatus(4)
        == OrderStatus::REJECTED,
        "Invalid order gets REJECTED status"
    );


    // --------------------------------------------------------
    // UNKNOWN
    // --------------------------------------------------------

    check(
        book.getOrderStatus(999)
        == OrderStatus::UNKNOWN,
        "Unknown order gets UNKNOWN status"
    );
}


// ============================================================
// TEST 16: MARKET ORDER STATUS
// ============================================================

void testMarketOrderStatus() {

    OrderBook book;


    // --------------------------------------------------------
    // No liquidity
    // --------------------------------------------------------

    book.addOrder({
        1,
        Side::BUY,
        OrderType::MARKET,
        0,
        100,
        1
    });

    check(
        book.getOrderStatus(1)
        == OrderStatus::CANCELLED,
        "Unfilled market order gets CANCELLED status"
    );


    // --------------------------------------------------------
    // Add liquidity
    // --------------------------------------------------------

    book.addOrder({
        2,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        2
    });


    // --------------------------------------------------------
    // Partially fill market order
    // --------------------------------------------------------

    book.addOrder({
        3,
        Side::BUY,
        OrderType::MARKET,
        0,
        100,
        3
    });

    check(
        book.getOrderStatus(3)
        == OrderStatus::PARTIALLY_FILLED,
        "Partially filled market order gets PARTIALLY_FILLED status"
    );
}


// ============================================================
// TEST 17: IOC
// ============================================================

void testIOC() {

    OrderBook book;


    // --------------------------------------------------------
    // Add 50 SELL liquidity
    // --------------------------------------------------------

    book.addOrder({
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        1
    });


    // --------------------------------------------------------
    // IOC BUY wants 100
    //
    // 50 gets executed.
    // Remaining 50 must be cancelled.
    // --------------------------------------------------------

    book.addOrder({
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        100,
        2,
        TimeInForce::IOC,
        false
    });


    check(
        book.getTradeCount() == 1,
        "IOC executes available quantity"
    );


    check(
        book.getOrderStatus(2)
        == OrderStatus::CANCELLED,
        "IOC cancels remaining quantity"
    );


    check(
        book.getBestBid() == -1,
        "IOC remainder is not stored"
    );


    check(
        book.getOrderQuantity(1) == -1,
        "IOC consumes available SELL liquidity"
    );
}


// ============================================================
// TEST 18: FOK SUCCESS
// ============================================================

void testFOKSuccess() {

    OrderBook book;


    // --------------------------------------------------------
    // Add 50 at 100
    // --------------------------------------------------------

    book.addOrder({
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        1
    });


    // --------------------------------------------------------
    // Add 50 at 101
    // --------------------------------------------------------

    book.addOrder({
        2,
        Side::SELL,
        OrderType::LIMIT,
        101.0,
        50,
        2
    });


    // --------------------------------------------------------
    // FOK BUY wants exactly 100
    // --------------------------------------------------------

    book.addOrder({
        3,
        Side::BUY,
        OrderType::LIMIT,
        101.0,
        100,
        3,
        TimeInForce::FOK,
        false
    });


    check(
        book.getTradeCount() == 2,
        "FOK executes across multiple price levels"
    );


    check(
        book.getOrderStatus(3)
        == OrderStatus::FILLED,
        "FOK becomes FILLED when full quantity is available"
    );


    check(
        book.getBestAsk() == -1,
        "FOK consumes all required liquidity"
    );
}


// ============================================================
// TEST 19: FOK FAILURE
// ============================================================

void testFOKFailure() {

    OrderBook book;


    // --------------------------------------------------------
    // Only 50 available
    // --------------------------------------------------------

    book.addOrder({
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        1
    });


    // --------------------------------------------------------
    // FOK wants 100
    //
    // Should NOT execute 50.
    // Entire order must be cancelled.
    // --------------------------------------------------------

    book.addOrder({
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        100,
        2,
        TimeInForce::FOK,
        false
    });


    check(
        book.getTradeCount() == 0,
        "Failed FOK creates no trades"
    );


    check(
        book.getOrderStatus(2)
        == OrderStatus::CANCELLED,
        "Failed FOK is cancelled"
    );


    check(
        book.getOrderQuantity(1) == 50,
        "Failed FOK leaves existing liquidity untouched"
    );


    check(
        book.getBestAsk() == 100.0,
        "Failed FOK does not remove the best ask"
    );
}


// ============================================================
// TEST 20: POST ONLY
// ============================================================

void testPostOnly() {

    OrderBook book;


    // --------------------------------------------------------
    // Existing SELL at 100
    // --------------------------------------------------------

    book.addOrder({
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        1
    });


    // --------------------------------------------------------
    // POST_ONLY BUY at 101
    //
    // This would immediately cross the SELL.
    // Therefore it must be rejected.
    // --------------------------------------------------------

    book.addOrder({
        2,
        Side::BUY,
        OrderType::LIMIT,
        101.0,
        50,
        2,
        TimeInForce::GTC,
        true
    });


    check(
        book.getTradeCount() == 0,
        "POST_ONLY prevents immediate execution"
    );


    check(
        book.getOrderStatus(2)
        == OrderStatus::REJECTED,
        "Crossing POST_ONLY order is rejected"
    );


    check(
        book.getBestBid() == -1,
        "Rejected POST_ONLY order is not stored"
    );


    check(
        book.getBestAsk() == 100.0,
        "Existing SELL remains untouched"
    );
}


// ============================================================
// TEST 21: POST ONLY NON-CROSSING
// ============================================================

void testPostOnlyNonCrossing() {

    OrderBook book;


    // Existing SELL at 105

    book.addOrder({
        1,
        Side::SELL,
        OrderType::LIMIT,
        105.0,
        50,
        1
    });


    // POST_ONLY BUY at 100
    //
    // Does not cross.
    // Therefore it should rest in the book.

    book.addOrder({
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        2,
        TimeInForce::GTC,
        true
    });


    check(
        book.getTradeCount() == 0,
        "Non-crossing POST_ONLY creates no trade"
    );


    check(
        book.getOrderStatus(2)
        == OrderStatus::OPEN,
        "Non-crossing POST_ONLY order becomes OPEN"
    );


    check(
        book.getBestBid() == 100.0,
        "Non-crossing POST_ONLY order rests in book"
    );
}


// ============================================================
// TEST 22: FOK WITH INSUFFICIENT PRICE LIQUIDITY
// ============================================================

void testFOKPriceConstraint() {

    OrderBook book;


    // 50 available at 100

    book.addOrder({
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        1
    });


    // Another 50 exists, but at 105

    book.addOrder({
        2,
        Side::SELL,
        OrderType::LIMIT,
        105.0,
        50,
        2
    });


    // BUY only accepts prices <= 100.
    //
    // It needs 100 but only 50 is available
    // at acceptable prices.

    book.addOrder({
        3,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        100,
        3,
        TimeInForce::FOK,
        false
    });


    check(
        book.getTradeCount() == 0,
        "FOK respects limit price"
    );


    check(
        book.getOrderStatus(3)
        == OrderStatus::CANCELLED,
        "FOK is cancelled when enough price-compatible liquidity is unavailable"
    );


    check(
        book.getOrderQuantity(1) == 50,
        "FOK failure leaves compatible liquidity untouched"
    );


    check(
        book.getOrderQuantity(2) == 50,
        "FOK failure does not consume worse-priced liquidity"
    );
}


// ============================================================
// MAIN TEST RUNNER
// ============================================================

int main() {

    cout << "\n";

    cout << "========================================\n";
    cout << "       ORDER BOOK TEST SUITE\n";
    cout << "========================================\n\n";


    // Original tests

    testBasicMatching();

    testPartialFill();

    testPricePriority();

    testTimePriority();

    testMarketBuy();

    testMarketSell();

    testCancellation();

    testModification();

    testModificationPriority();

    testInvalidQuantity();

    testInvalidPrice();

    testDuplicateID();

    testInvalidCancellation();

    testInvalidModification();

    testOrderStatuses();

    testMarketOrderStatus();


    // New execution-policy tests

    testIOC();

    testFOKSuccess();

    testFOKFailure();

    testPostOnly();

    testPostOnlyNonCrossing();

    testFOKPriceConstraint();


    // --------------------------------------------------------
    // RESULTS
    // --------------------------------------------------------

    cout << "\n========================================\n";
    cout << "TEST RESULTS\n";
    cout << "========================================\n";

    cout << "Passed: "
         << passed
         << "\n";

    cout << "Failed: "
         << failed
         << "\n";


    if (failed == 0) {

        cout << "\nALL TESTS PASSED.\n";

        return 0;
    }


    cout << "\nSOME TESTS FAILED.\n";

    return 1;
}