#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include <vector>
#include <string>

#include "../src/OrderBook.h"
#include "ExecutionReport.h"


class MatchingEngine
{
private:

    OrderBook orderBook;

    std::vector<ExecutionReport> executionReports;


    // ========================================================
    // ADD EXECUTION REPORT
    // ========================================================

    void addExecutionReport(
        int orderId,
        ExecutionType type,
        int quantity,
        double price,
        const std::string& message
    );


    // ========================================================
    // PROCESS TRADES CREATED BY AN ORDER
    // ========================================================

    void processNewTrades(
        std::size_t previousTradeCount,
        int incomingOrderId
    );


public:

    // ========================================================
    // ORDER OPERATIONS
    // ========================================================

    bool submitOrder(Order order);

    bool cancelOrder(int orderId);

    bool modifyOrder(
        int orderId,
        double newPrice,
        int newQuantity
    );


    // ========================================================
    // ORDER BOOK
    // ========================================================

    const OrderBook& getOrderBook() const;

    void printBook() const;

    void printTradeHistory() const;


    // ========================================================
    // MARKET DATA
    // ========================================================

    double getBestBid() const;

    double getBestAsk() const;


    // ========================================================
    // ORDER INFORMATION
    // ========================================================

    int getOrderQuantity(int orderId) const;

    OrderStatus getOrderStatus(int orderId) const;


    // ========================================================
    // TRADE STATISTICS
    // ========================================================

    int getTradeCount() const;


    // ========================================================
    // EXECUTION REPORTS
    // ========================================================

    const std::vector<ExecutionReport>&
    getExecutionReports() const;

    void printExecutionReports() const;
};

#endif