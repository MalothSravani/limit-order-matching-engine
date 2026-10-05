#include "MatchingEngine.h"

#include <iostream>

using namespace std;


// ============================================================
// ADD EXECUTION REPORT
// ============================================================

void MatchingEngine::addExecutionReport(
    int orderId,
    ExecutionType type,
    int quantity,
    double price,
    const string& message
)
{
    ExecutionReport report;

    report.orderId = orderId;
    report.type = type;
    report.quantity = quantity;
    report.price = price;
    report.message = message;

    executionReports.push_back(report);
}


// ============================================================
// PROCESS NEW TRADES
// ============================================================

void MatchingEngine::processNewTrades(
    size_t previousTradeCount,
    int incomingOrderId
)
{
    const vector<Trade>& trades =
        orderBook.getTradeHistory();

    if (previousTradeCount >= trades.size())
    {
        return;
    }

    // Process only trades created by this submission
    for (size_t i = previousTradeCount; i < trades.size(); ++i)
    {
        const Trade& trade = trades[i];

        // ====================================================
        // BUY SIDE EXECUTION
        // ====================================================

        OrderStatus buyStatus =
            orderBook.getOrderStatus(trade.buyOrderId);

        if (buyStatus == OrderStatus::FILLED)
        {
            addExecutionReport(
                trade.buyOrderId,
                ExecutionType::FILLED,
                trade.quantity,
                trade.price,
                "BUY order filled"
            );
        }
        else
        {
            addExecutionReport(
                trade.buyOrderId,
                ExecutionType::PARTIALLY_FILLED,
                trade.quantity,
                trade.price,
                "BUY order partially filled"
            );
        }


        // ====================================================
        // SELL SIDE EXECUTION
        // ====================================================

        OrderStatus sellStatus =
            orderBook.getOrderStatus(trade.sellOrderId);

        if (sellStatus == OrderStatus::FILLED)
        {
            addExecutionReport(
                trade.sellOrderId,
                ExecutionType::FILLED,
                trade.quantity,
                trade.price,
                "SELL order filled"
            );
        }
        else
        {
            addExecutionReport(
                trade.sellOrderId,
                ExecutionType::PARTIALLY_FILLED,
                trade.quantity,
                trade.price,
                "SELL order partially filled"
            );
        }
    }
}


// ============================================================
// SUBMIT ORDER
// ============================================================

bool MatchingEngine::submitOrder(Order order)
{
    // Save trade count BEFORE submission
    size_t previousTradeCount =
        orderBook.getTradeHistory().size();


    // ========================================================
    // SUBMIT TO ORDER BOOK
    // ========================================================

    bool accepted =
        orderBook.addOrder(order);


    // ========================================================
    // REJECTED
    // ========================================================

    if (!accepted)
    {
        addExecutionReport(
            order.id,
            ExecutionType::REJECTED,
            0,
            order.price,
            "Order rejected"
        );

        return false;
    }


    // ========================================================
    // PROCESS ACTUAL TRADES
    // ========================================================

    processNewTrades(
        previousTradeCount,
        order.id
    );


    // ========================================================
    // FINAL ORDER STATUS
    // ========================================================

    OrderStatus status =
        orderBook.getOrderStatus(order.id);


    // ========================================================
    // OPEN
    // ========================================================

    if (status == OrderStatus::OPEN)
    {
        addExecutionReport(
            order.id,
            ExecutionType::ACCEPTED,
            order.quantity,
            order.price,
            "Order accepted and resting in book"
        );

        return true;
    }


    // ========================================================
    // CANCELLED
    // ========================================================

    if (status == OrderStatus::CANCELLED)
    {
        addExecutionReport(
            order.id,
            ExecutionType::CANCELLED,
            0,
            order.price,
            "Remaining quantity cancelled"
        );

        return true;
    }


    // ========================================================
    // FILLED / PARTIALLY FILLED
    //
    // These reports were already generated from actual trades.
    // ========================================================

    return true;
}


// ============================================================
// CANCEL ORDER
// ============================================================

bool MatchingEngine::cancelOrder(int orderId)
{
    bool result =
        orderBook.cancelOrder(orderId);


    if (result)
    {
        addExecutionReport(
            orderId,
            ExecutionType::CANCELLED,
            0,
            0.0,
            "Order cancelled successfully"
        );
    }
    else
    {
        addExecutionReport(
            orderId,
            ExecutionType::REJECTED,
            0,
            0.0,
            "Cancellation failed"
        );
    }


    return result;
}


// ============================================================
// MODIFY ORDER
// ============================================================

bool MatchingEngine::modifyOrder(
    int orderId,
    double newPrice,
    int newQuantity
)
{
    bool result =
        orderBook.modifyOrder(
            orderId,
            newPrice,
            newQuantity
        );


    if (result)
    {
        addExecutionReport(
            orderId,
            ExecutionType::MODIFIED,
            newQuantity,
            newPrice,
            "Order modified successfully"
        );
    }
    else
    {
        addExecutionReport(
            orderId,
            ExecutionType::REJECTED,
            0,
            newPrice,
            "Order modification failed"
        );
    }


    return result;
}


// ============================================================
// GET ORDER BOOK
// ============================================================

const OrderBook&
MatchingEngine::getOrderBook() const
{
    return orderBook;
}


// ============================================================
// PRINT BOOK
// ============================================================

void MatchingEngine::printBook() const
{
    orderBook.printBook();
}


// ============================================================
// PRINT TRADE HISTORY
// ============================================================

void MatchingEngine::printTradeHistory() const
{
    orderBook.printTradeHistory();
}


// ============================================================
// BEST BID
// ============================================================

double MatchingEngine::getBestBid() const
{
    return orderBook.getBestBid();
}


// ============================================================
// BEST ASK
// ============================================================

double MatchingEngine::getBestAsk() const
{
    return orderBook.getBestAsk();
}


// ============================================================
// ORDER QUANTITY
// ============================================================

int MatchingEngine::getOrderQuantity(
    int orderId
) const
{
    return orderBook.getOrderQuantity(
        orderId
    );
}


// ============================================================
// ORDER STATUS
// ============================================================

OrderStatus MatchingEngine::getOrderStatus(
    int orderId
) const
{
    return orderBook.getOrderStatus(
        orderId
    );
}


// ============================================================
// TRADE COUNT
// ============================================================

int MatchingEngine::getTradeCount() const
{
    return orderBook.getTradeCount();
}


// ============================================================
// GET EXECUTION REPORTS
// ============================================================

const vector<ExecutionReport>&
MatchingEngine::getExecutionReports() const
{
    return executionReports;
}


// ============================================================
// PRINT EXECUTION REPORTS
// ============================================================

void MatchingEngine::printExecutionReports() const
{
    cout << "\n";
    cout << "========================================\n";
    cout << "        EXECUTION REPORTS\n";
    cout << "========================================\n";


    if (executionReports.empty())
    {
        cout << "No execution reports.\n";

        cout << "\n========================================\n";

        return;
    }


    for (const auto& report :
         executionReports)
    {
        cout << "\n";


        cout << "Order ID : "
             << report.orderId
             << "\n";


        cout << "Type     : ";


        switch (report.type)
        {
            case ExecutionType::ACCEPTED:

                cout << "ACCEPTED";

                break;


            case ExecutionType::FILLED:

                cout << "FILLED";

                break;


            case ExecutionType::PARTIALLY_FILLED:

                cout << "PARTIALLY_FILLED";

                break;


            case ExecutionType::CANCELLED:

                cout << "CANCELLED";

                break;


            case ExecutionType::REJECTED:

                cout << "REJECTED";

                break;


            case ExecutionType::MODIFIED:

                cout << "MODIFIED";

                break;
        }


        cout << "\n";


        cout << "Quantity : "
             << report.quantity
             << "\n";


        cout << "Price    : "
             << report.price
             << "\n";


        cout << "Message  : "
             << report.message
             << "\n";
    }


    cout << "\n========================================\n";
}