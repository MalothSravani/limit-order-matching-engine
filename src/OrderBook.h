#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <map>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstddef>

#include "../include/Order.h"
#include "Trade.h"


class OrderBook {

private:

    // ========================================================
    // NODE INDEX
    // ========================================================

    using NodeIndex = std::size_t;

    static constexpr NodeIndex INVALID_INDEX =
        static_cast<NodeIndex>(-1);


    // ========================================================
    // ORDER NODE
    // ========================================================

    struct OrderNode {

        Order order;

        NodeIndex prev = INVALID_INDEX;

        NodeIndex next = INVALID_INDEX;

        bool active = true;
    };


    // ========================================================
    // PRICE LEVEL
    // ========================================================

    struct PriceLevel {

        NodeIndex head = INVALID_INDEX;

        NodeIndex tail = INVALID_INDEX;

        std::size_t size = 0;
    };


    // ========================================================
    // ORDER BOOK
    // ========================================================

    std::map<
        double,
        PriceLevel,
        std::greater<double>
    > bids;


    std::map<
        double,
        PriceLevel
    > asks;


    // ========================================================
    // CONTIGUOUS ORDER STORAGE
    // ========================================================

    std::vector<OrderNode> orderPool;


    // ========================================================
    // ORDER ID -> NODE
    // ========================================================

    std::unordered_map<
        int,
        NodeIndex
    > orderIndex;


    // ========================================================
    // ORDER STATUS
    // ========================================================

    std::unordered_map<
        int,
        OrderStatus
    > orderStatus;


    // ========================================================
    // FREE NODE STORAGE
    // ========================================================

    std::vector<NodeIndex> freeNodes;


    // ========================================================
    // TRADE HISTORY
    // ========================================================

    std::vector<Trade> tradeHistory;


    // ========================================================
    // TIMESTAMP
    // ========================================================

    long long currentTimestamp = 1000;


    // ========================================================
    // INTERNAL NODE FUNCTIONS
    // ========================================================

    NodeIndex createNode(
        const Order& order
    );


    void releaseNode(
        NodeIndex index
    );


    void addNodeToPriceLevel(
        PriceLevel& level,
        NodeIndex node
    );


    void removeNodeFromPriceLevel(
        PriceLevel& level,
        NodeIndex node
    );


    // ========================================================
    // MATCHING
    // ========================================================

    void matchOrder(
        Order& incomingOrder
    );


    void recordTrade(
        const Order& buyOrder,
        const Order& sellOrder,
        double tradePrice,
        int tradeQuantity
    );


    // ========================================================
    // VALIDATION
    // ========================================================

    bool validateOrder(
        const Order& order
    ) const;


    // ========================================================
    // SPECIAL ORDER TYPES
    // ========================================================

    bool canFullyFill(
        const Order& order
    ) const;


    bool wouldImmediatelyTrade(
        const Order& order
    ) const;


    void cancelRemainingOrder(
        int orderId
    );


public:

    // ========================================================
    // CONSTRUCTOR
    // ========================================================

    OrderBook();


    // ========================================================
    // ORDER OPERATIONS
    // ========================================================

    bool addOrder(
        Order order
    );


    bool cancelOrder(
        int orderId
    );


    bool modifyOrder(
        int orderId,
        double newPrice,
        int newQuantity
    );


    // ========================================================
    // DISPLAY
    // ========================================================

    void printBook() const;

    void printTradeHistory() const;


    // ========================================================
    // QUERIES
    // ========================================================

    double getBestBid() const;

    double getBestAsk() const;


    int getOrderQuantity(
        int orderId
    ) const;


    int getTradeCount() const;


    OrderStatus getOrderStatus(
        int orderId
    ) const;


    // ========================================================
    // TRADE HISTORY ACCESS
    // ========================================================

    const std::vector<Trade>& getTradeHistory() const;
};


#endif