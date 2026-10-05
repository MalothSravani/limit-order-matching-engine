#include <iostream>
#include <string>

#include "MatchingEngine.h"

using namespace std;


// ============================================================
// ORDER STATUS TO STRING
// ============================================================

string statusToString(
    OrderStatus status
)
{
    switch (status)
    {
        case OrderStatus::OPEN:

            return "OPEN";


        case OrderStatus::PARTIALLY_FILLED:

            return "PARTIALLY_FILLED";


        case OrderStatus::FILLED:

            return "FILLED";


        case OrderStatus::CANCELLED:

            return "CANCELLED";


        case OrderStatus::REJECTED:

            return "REJECTED";


        case OrderStatus::UNKNOWN:

            return "UNKNOWN";
    }


    return "UNKNOWN";
}


// ============================================================
// MAIN
// ============================================================

int main()
{
    cout << "\n";

    cout << "========================================\n";
    cout << "       MATCHING ENGINE DEMO\n";
    cout << "========================================\n";


    MatchingEngine engine;


    // ========================================================
    // SELL ORDER
    // ========================================================

    cout << "\nAdding SELL order...\n";


    Order sellOrder;

    sellOrder.id = 1;

    sellOrder.side = Side::SELL;

    sellOrder.type = OrderType::LIMIT;

    sellOrder.price = 100.0;

    sellOrder.quantity = 50;

    sellOrder.timestamp = 1;

    sellOrder.timeInForce =
        TimeInForce::GTC;

    sellOrder.postOnly = false;


    engine.submitOrder(
        sellOrder
    );


    // ========================================================
    // BUY ORDER
    // ========================================================

    cout << "\nAdding BUY order...\n";


    Order buyOrder;

    buyOrder.id = 2;

    buyOrder.side = Side::BUY;

    buyOrder.type = OrderType::LIMIT;

    buyOrder.price = 100.0;

    buyOrder.quantity = 30;

    buyOrder.timestamp = 2;

    buyOrder.timeInForce =
        TimeInForce::GTC;

    buyOrder.postOnly = false;


    engine.submitOrder(
        buyOrder
    );


    // ========================================================
    // MARKET DATA
    // ========================================================

    cout << "\n";
    cout << "========================================\n";
    cout << "          MARKET DATA\n";
    cout << "========================================\n";


    cout << "Best Bid : "
         << engine.getBestBid()
         << "\n";


    cout << "Best Ask : "
         << engine.getBestAsk()
         << "\n";


    // ========================================================
    // ORDER STATUS
    // ========================================================

    cout << "\n";
    cout << "========================================\n";
    cout << "          ORDER STATUS\n";
    cout << "========================================\n";


    cout << "Order 1 : "
         << statusToString(
                engine.getOrderStatus(1)
            )
         << "\n";


    cout << "Order 2 : "
         << statusToString(
                engine.getOrderStatus(2)
            )
         << "\n";


    // ========================================================
    // TRADE COUNT
    // ========================================================

    cout << "\n";

    cout << "Trades executed : "
         << engine.getTradeCount()
         << "\n";


    // ========================================================
    // ORDER BOOK
    // ========================================================

    engine.printBook();


    // ========================================================
    // TRADE HISTORY
    // ========================================================

    engine.printTradeHistory();


    // ========================================================
    // EXECUTION REPORTS
    // ========================================================

    engine.printExecutionReports();


    // ========================================================
    // COMPLETED
    // ========================================================

    cout << "\n";

    cout << "========================================\n";
    cout << "       DEMO COMPLETED\n";
    cout << "========================================\n";


    return 0;
}