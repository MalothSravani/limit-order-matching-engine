#include <iostream>
#include <string>
#include <cmath>

#include "../include/Order.h"
#include "../src/OrderBook.h"
#include "../include/MatchingEngine.h"

using namespace std;


// ============================================================
// TEST COUNTERS
// ============================================================

int passed = 0;
int failed = 0;


// ============================================================
// TEST HELPER
// ============================================================

void test(bool condition, const string& testName)
{
    if (condition)
    {
        cout << "[PASS] " << testName << "\n";
        passed++;
    }
    else
    {
        cout << "[FAIL] " << testName << "\n";
        failed++;
    }
}


bool equalDouble(double a, double b)
{
    return fabs(a - b) < 1e-9;
}


// ============================================================
// 1. BASIC BUY ORDER
// ============================================================

void testBasicBuyOrder()
{
    OrderBook book;

    Order order{
        1,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    bool result = book.addOrder(order);

    test(result, "Basic BUY order accepted");

    test(
        book.getOrderQuantity(1) == 50,
        "Basic BUY quantity correct"
    );

    test(
        book.getOrderStatus(1) == OrderStatus::OPEN,
        "Basic BUY status OPEN"
    );

    test(
        equalDouble(book.getBestBid(), 100.0),
        "Best bid correct"
    );
}


// ============================================================
// 2. BASIC SELL ORDER
// ============================================================

void testBasicSellOrder()
{
    OrderBook book;

    Order order{
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    bool result = book.addOrder(order);

    test(result, "Basic SELL order accepted");

    test(
        book.getOrderQuantity(1) == 50,
        "SELL quantity correct"
    );

    test(
        book.getOrderStatus(1) == OrderStatus::OPEN,
        "SELL status OPEN"
    );

    test(
        equalDouble(book.getBestAsk(), 100.0),
        "Best ask correct"
    );
}


// ============================================================
// 3. FULL MATCH
// ============================================================

void testFullMatch()
{
    OrderBook book;

    Order sell{
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    Order buy{
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    book.addOrder(sell);
    book.addOrder(buy);

    test(
        book.getOrderStatus(1) == OrderStatus::FILLED,
        "SELL fully filled"
    );

    test(
        book.getOrderStatus(2) == OrderStatus::FILLED,
        "BUY fully filled"
    );

    test(
        book.getTradeCount() == 1,
        "One trade created"
    );

    test(
        book.getBestAsk() == -1,
        "No ask after full match"
    );

    test(
        book.getBestBid() == -1,
        "No bid after full match"
    );
}


// ============================================================
// 4. PARTIAL MATCH
// ============================================================

void testPartialMatch()
{
    OrderBook book;

    Order sell{
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    Order buy{
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        30,
        0
    };

    book.addOrder(sell);
    book.addOrder(buy);

    test(
        book.getOrderStatus(1) == OrderStatus::PARTIALLY_FILLED,
        "SELL partially filled"
    );

    test(
        book.getOrderStatus(2) == OrderStatus::FILLED,
        "BUY fully filled"
    );

    test(
        book.getOrderQuantity(1) == 20,
        "Remaining SELL quantity is 20"
    );

    test(
        book.getTradeCount() == 1,
        "Partial match creates one trade"
    );
}


// ============================================================
// 5. PRICE PRIORITY
// ============================================================

void testPricePriority()
{
    OrderBook book;

    Order sell1{
        1,
        Side::SELL,
        OrderType::LIMIT,
        105.0,
        50,
        0
    };

    Order sell2{
        2,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    Order buy{
        3,
        Side::BUY,
        OrderType::LIMIT,
        105.0,
        50,
        0
    };

    book.addOrder(sell1);
    book.addOrder(sell2);
    book.addOrder(buy);

    test(
        book.getOrderStatus(2) == OrderStatus::FILLED,
        "Best ask gets matched first"
    );

    test(
        book.getOrderStatus(1) == OrderStatus::OPEN,
        "Higher ask remains open"
    );
}


// ============================================================
// 6. TIME PRIORITY
// ============================================================

void testTimePriority()
{
    OrderBook book;

    Order sell1{
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        1
    };

    Order sell2{
        2,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        2
    };

    Order buy{
        3,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        3
    };

    book.addOrder(sell1);
    book.addOrder(sell2);
    book.addOrder(buy);

    test(
        book.getOrderStatus(1) == OrderStatus::FILLED,
        "Earlier order gets time priority"
    );

    test(
        book.getOrderStatus(2) == OrderStatus::OPEN,
        "Later order remains open"
    );
}


// ============================================================
// 7. CANCEL ORDER
// ============================================================

void testCancellation()
{
    OrderBook book;

    Order order{
        1,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    book.addOrder(order);

    bool result = book.cancelOrder(1);

    test(
        result,
        "Order cancellation succeeds"
    );

    test(
        book.getOrderStatus(1) == OrderStatus::CANCELLED,
        "Cancelled order status correct"
    );

    test(
        book.getOrderQuantity(1) == 0,
        "Cancelled order quantity is zero"
    );

    test(
        book.getBestBid() == -1,
        "Cancelled order removed from book"
    );
}


// ============================================================
// 8. INVALID CANCELLATION
// ============================================================

void testInvalidCancellation()
{
    OrderBook book;

    bool result = book.cancelOrder(999);

    test(
        !result,
        "Cancelling unknown order fails"
    );
}


// ============================================================
// 9. MODIFY ORDER
// ============================================================

void testModifyOrder()
{
    OrderBook book;

    Order order{
        1,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    book.addOrder(order);

    bool result =
        book.modifyOrder(
            1,
            105.0,
            75
        );

    test(
        result,
        "Order modification succeeds"
    );

    test(
        book.getOrderQuantity(1) == 75,
        "Modified quantity correct"
    );

    test(
        equalDouble(book.getBestBid(), 105.0),
        "Modified price correct"
    );
}


// ============================================================
// 10. MARKET ORDER
// ============================================================

void testMarketOrder()
{
    OrderBook book;

    Order sell{
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    Order marketBuy{
        2,
        Side::BUY,
        OrderType::MARKET,
        0.0,
        30,
        0,
        TimeInForce::IOC,
        false
    };

    book.addOrder(sell);
    book.addOrder(marketBuy);

    test(
        book.getOrderStatus(2) == OrderStatus::FILLED ||
        book.getOrderStatus(2) == OrderStatus::PARTIALLY_FILLED,
        "Market BUY executes"
    );

    test(
        book.getTradeCount() >= 1,
        "Market order creates trade"
    );
}


// ============================================================
// 11. IOC ORDER
// ============================================================

void testIOC()
{
    OrderBook book;

    Order sell{
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        20,
        0
    };

    Order buy{
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        0,
        TimeInForce::IOC,
        false
    };

    book.addOrder(sell);
    book.addOrder(buy);

    test(
        book.getTradeCount() == 1,
        "IOC executes available quantity"
    );

    test(
        book.getOrderStatus(2) != OrderStatus::OPEN,
        "IOC does not remain resting"
    );
}


// ============================================================
// 12. FOK SUCCESS
// ============================================================

void testFOKSuccess()
{
    OrderBook book;

    Order sell{
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    Order buy{
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        0,
        TimeInForce::FOK,
        false
    };

    book.addOrder(sell);
    book.addOrder(buy);

    test(
        book.getOrderStatus(2) == OrderStatus::FILLED,
        "FOK fills when full quantity available"
    );

    test(
        book.getTradeCount() == 1,
        "Successful FOK creates trade"
    );
}


// ============================================================
// 13. FOK FAILURE
// ============================================================

void testFOKFailure()
{
    OrderBook book;

    Order sell{
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        30,
        0
    };

    Order buy{
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        0,
        TimeInForce::FOK,
        false
    };

    book.addOrder(sell);
    book.addOrder(buy);

    test(
        book.getOrderStatus(2) != OrderStatus::FILLED,
        "FOK does not fill without enough liquidity"
    );

    test(
        book.getTradeCount() == 0,
        "Failed FOK creates no trade"
    );
}


// ============================================================
// 14. POST ONLY
// ============================================================

void testPostOnly()
{
    OrderBook book;

    Order sell{
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    book.addOrder(sell);

    Order buy{
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        30,
        0,
        TimeInForce::GTC,
        true
    };

    bool result = book.addOrder(buy);

    test(
        !result,
        "POST_ONLY crossing order rejected"
    );

    test(
        book.getTradeCount() == 0,
        "POST_ONLY creates no trade"
    );
}


// ============================================================
// 15. TRADE HISTORY
// ============================================================

void testTradeHistory()
{
    OrderBook book;

    Order sell{
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    Order buy{
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        30,
        0
    };

    book.addOrder(sell);
    book.addOrder(buy);

    const auto& trades =
        book.getTradeHistory();

    test(
        trades.size() == 1,
        "Trade history contains one trade"
    );

    if (!trades.empty())
    {
        test(
            trades[0].buyOrderId == 2,
            "Trade BUY ID correct"
        );

        test(
            trades[0].sellOrderId == 1,
            "Trade SELL ID correct"
        );

        test(
            trades[0].quantity == 30,
            "Trade quantity correct"
        );

        test(
            equalDouble(trades[0].price, 100.0),
            "Trade price correct"
        );
    }
}


// ============================================================
// 16. MATCHING ENGINE
// ============================================================

void testMatchingEngine()
{
    MatchingEngine engine;

    Order sell{
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    Order buy{
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        30,
        0
    };

    bool sellAccepted =
        engine.submitOrder(sell);

    bool buyAccepted =
        engine.submitOrder(buy);

    test(
        sellAccepted,
        "MatchingEngine accepts SELL"
    );

    test(
        buyAccepted,
        "MatchingEngine accepts BUY"
    );

    test(
        engine.getTradeCount() == 1,
        "MatchingEngine records trade"
    );

    test(
        engine.getOrderStatus(1) ==
        OrderStatus::PARTIALLY_FILLED,
        "MatchingEngine SELL status correct"
    );

    test(
        engine.getOrderStatus(2) ==
        OrderStatus::FILLED,
        "MatchingEngine BUY status correct"
    );
}


// ============================================================
// 17. MATCHING ENGINE CANCEL
// ============================================================

void testMatchingEngineCancel()
{
    MatchingEngine engine;

    Order order{
        1,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    engine.submitOrder(order);

    bool result =
        engine.cancelOrder(1);

    test(
        result,
        "MatchingEngine cancellation succeeds"
    );

    test(
        engine.getOrderStatus(1) ==
        OrderStatus::CANCELLED,
        "MatchingEngine cancellation status correct"
    );
}


// ============================================================
// 18. MATCHING ENGINE MODIFY
// ============================================================

void testMatchingEngineModify()
{
    MatchingEngine engine;

    Order order{
        1,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    engine.submitOrder(order);

    bool result =
        engine.modifyOrder(
            1,
            110.0,
            75
        );

    test(
        result,
        "MatchingEngine modification succeeds"
    );

    test(
        engine.getOrderQuantity(1) == 75,
        "MatchingEngine modified quantity correct"
    );

    test(
        equalDouble(
            engine.getBestBid(),
            110.0
        ),
        "MatchingEngine modified price correct"
    );
}


// ============================================================
// 19. EXECUTION REPORTS
// ============================================================

void testExecutionReports()
{
    MatchingEngine engine;

    Order sell{
        1,
        Side::SELL,
        OrderType::LIMIT,
        100.0,
        50,
        0
    };

    Order buy{
        2,
        Side::BUY,
        OrderType::LIMIT,
        100.0,
        30,
        0
    };

    engine.submitOrder(sell);
    engine.submitOrder(buy);

    const auto& reports =
        engine.getExecutionReports();

    test(
        !reports.empty(),
        "Execution reports generated"
    );

    test(
        reports.size() >= 3,
        "Expected execution reports generated"
    );
}


// ============================================================
// 20. NON-CROSSING ORDERS
// ============================================================

void testNoCrossingOrders()
{
    OrderBook book;

    Order buy{
        1,
        Side::BUY,
        OrderType::LIMIT,
        95.0,
        50,
        0
    };

    Order sell{
        2,
        Side::SELL,
        OrderType::LIMIT,
        105.0,
        50,
        0
    };

    book.addOrder(buy);
    book.addOrder(sell);

    test(
        book.getTradeCount() == 0,
        "Non-crossing orders create no trade"
    );

    test(
        book.getOrderStatus(1) == OrderStatus::OPEN,
        "Non-crossing BUY remains open"
    );

    test(
        book.getOrderStatus(2) == OrderStatus::OPEN,
        "Non-crossing SELL remains open"
    );
}


// ============================================================
// RUN ALL TESTS
// ============================================================

int main()
{
    cout << "\n";
    cout << "========================================\n";
    cout << "       LIMIT ORDER BOOK TEST SUITE\n";
    cout << "========================================\n\n";


    testBasicBuyOrder();
    testBasicSellOrder();

    testFullMatch();
    testPartialMatch();

    testPricePriority();
    testTimePriority();

    testCancellation();
    testInvalidCancellation();

    testModifyOrder();

    testMarketOrder();

    testIOC();

    testFOKSuccess();
    testFOKFailure();

    testPostOnly();

    testTradeHistory();

    testMatchingEngine();

    testMatchingEngineCancel();
    testMatchingEngineModify();

    testExecutionReports();

    testNoCrossingOrders();


    cout << "\n";
    cout << "========================================\n";
    cout << "             TEST RESULTS\n";
    cout << "========================================\n";

    cout << "Passed : "
         << passed
         << "\n";

    cout << "Failed : "
         << failed
         << "\n";

    cout << "Total  : "
         << passed + failed
         << "\n";

    cout << "========================================\n";


    if (failed == 0)
    {
        cout << "\nALL TESTS PASSED!\n";
        return 0;
    }

    cout << "\nSOME TESTS FAILED!\n";

    return 1;
}