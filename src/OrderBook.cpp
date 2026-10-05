#include "OrderBook.h"

#include <iostream>
#include <algorithm>

using namespace std;


// ============================================================
// CONSTRUCTOR
// ============================================================

OrderBook::OrderBook()
{
    orderPool.reserve(500000);

    freeNodes.reserve(100000);

    orderIndex.max_load_factor(0.70f);
    orderIndex.reserve(300000);

    orderStatus.max_load_factor(0.70f);
    orderStatus.reserve(300000);

    tradeHistory.reserve(300000);
}


// ============================================================
// CREATE NODE
// ============================================================

OrderBook::NodeIndex
OrderBook::createNode(
    const Order& order
)
{
    if (!freeNodes.empty())
    {
        NodeIndex index =
            freeNodes.back();

        freeNodes.pop_back();

        OrderNode& node =
            orderPool[index];

        node.order = order;

        node.prev = INVALID_INDEX;
        node.next = INVALID_INDEX;
        node.active = true;

        return index;
    }

    NodeIndex index =
        orderPool.size();

    orderPool.push_back(
        OrderNode{
            order,
            INVALID_INDEX,
            INVALID_INDEX,
            true
        }
    );

    return index;
}


// ============================================================
// RELEASE NODE
// ============================================================

void OrderBook::releaseNode(
    NodeIndex index
)
{
    if (index == INVALID_INDEX)
    {
        return;
    }

    orderPool[index].active = false;

    freeNodes.push_back(index);
}


// ============================================================
// ADD NODE TO PRICE LEVEL
// ============================================================

void OrderBook::addNodeToPriceLevel(
    PriceLevel& level,
    NodeIndex node
)
{
    OrderNode& current =
        orderPool[node];

    current.prev =
        INVALID_INDEX;

    current.next =
        INVALID_INDEX;

    if (
        level.head ==
        INVALID_INDEX
    )
    {
        level.head = node;
        level.tail = node;
        level.size = 1;

        return;
    }

    NodeIndex oldTail =
        level.tail;

    orderPool[oldTail].next =
        node;

    current.prev =
        oldTail;

    level.tail =
        node;

    ++level.size;
}


// ============================================================
// REMOVE NODE FROM PRICE LEVEL
// ============================================================

void OrderBook::removeNodeFromPriceLevel(
    PriceLevel& level,
    NodeIndex node
)
{
    OrderNode& current =
        orderPool[node];

    NodeIndex previous =
        current.prev;

    NodeIndex next =
        current.next;

    if (
        previous !=
        INVALID_INDEX
    )
    {
        orderPool[previous].next =
            next;
    }
    else
    {
        level.head =
            next;
    }

    if (
        next !=
        INVALID_INDEX
    )
    {
        orderPool[next].prev =
            previous;
    }
    else
    {
        level.tail =
            previous;
    }

    if (level.size > 0)
    {
        --level.size;
    }

    current.prev =
        INVALID_INDEX;

    current.next =
        INVALID_INDEX;
}


// ============================================================
// RECORD TRADE
// ============================================================

void OrderBook::recordTrade(
    const Order& buyOrder,
    const Order& sellOrder,
    double tradePrice,
    int tradeQuantity
)
{
    Trade trade;

    trade.buyOrderId =
        buyOrder.id;

    trade.sellOrderId =
        sellOrder.id;

    trade.price =
        tradePrice;

    trade.quantity =
        tradeQuantity;

    trade.timestamp =
        max(
            buyOrder.timestamp,
            sellOrder.timestamp
        );

    tradeHistory.push_back(
        trade
    );

#ifndef ORDERBOOK_SILENT

    cout << "\n===== TRADE EXECUTED =====\n";

    cout << "Buy Order ID: "
         << trade.buyOrderId
         << "\n";

    cout << "Sell Order ID: "
         << trade.sellOrderId
         << "\n";

    cout << "Trade Price: "
         << trade.price
         << "\n";

    cout << "Trade Quantity: "
         << trade.quantity
         << "\n";

#endif
}


// ============================================================
// VALIDATE ORDER
// ============================================================

bool OrderBook::validateOrder(
    const Order& order
) const
{
    // --------------------------------------------------------
    // ORDER ID
    // --------------------------------------------------------

    if (order.id <= 0)
    {
#ifndef ORDERBOOK_SILENT

        cout << "\n===== ORDER REJECTED =====\n";

        cout << "Invalid Order ID: "
             << order.id
             << "\n";

#endif

        return false;
    }


    // --------------------------------------------------------
    // DUPLICATE ACTIVE ORDER
    // --------------------------------------------------------

    if (
        orderIndex.find(order.id)
        != orderIndex.end()
    )
    {
#ifndef ORDERBOOK_SILENT

        cout << "\n===== ORDER REJECTED =====\n";

        cout << "Duplicate Order ID: "
             << order.id
             << "\n";

#endif

        return false;
    }


    // --------------------------------------------------------
    // QUANTITY
    // --------------------------------------------------------

    if (order.quantity <= 0)
    {
#ifndef ORDERBOOK_SILENT

        cout << "\n===== ORDER REJECTED =====\n";

        cout << "Invalid Quantity: "
             << order.quantity
             << "\n";

#endif

        return false;
    }


    // --------------------------------------------------------
    // LIMIT PRICE
    // --------------------------------------------------------

    if (
        order.type == OrderType::LIMIT &&
        order.price <= 0
    )
    {
#ifndef ORDERBOOK_SILENT

        cout << "\n===== ORDER REJECTED =====\n";

        cout << "Invalid LIMIT order price: "
             << order.price
             << "\n";

#endif

        return false;
    }


    // --------------------------------------------------------
    // MARKET PRICE
    // --------------------------------------------------------

    if (
        order.type == OrderType::MARKET &&
        order.price < 0
    )
    {
#ifndef ORDERBOOK_SILENT

        cout << "\n===== ORDER REJECTED =====\n";

        cout << "Invalid MARKET order price: "
             << order.price
             << "\n";

#endif

        return false;
    }


    // --------------------------------------------------------
    // POST ONLY + MARKET
    // --------------------------------------------------------

    if (
        order.postOnly &&
        order.type == OrderType::MARKET
    )
    {
#ifndef ORDERBOOK_SILENT

        cout << "\n===== ORDER REJECTED =====\n";

        cout
            << "POST_ONLY cannot be used with MARKET orders.\n";

#endif

        return false;
    }


    return true;
}


// ============================================================
// CHECK WHETHER ORDER CAN BE FULLY FILLED
// ============================================================

bool OrderBook::canFullyFill(
    const Order& order
) const
{
    int remaining =
        order.quantity;


    // ========================================================
    // BUY
    // ========================================================

    if (order.side == Side::BUY)
    {
        for (
            const auto& [price, level] :
            asks
        )
        {
            // LIMIT BUY cannot consume
            // asks above its limit price.

            if (
                order.type == OrderType::LIMIT &&
                order.price < price
            )
            {
                break;
            }

            NodeIndex node =
                level.head;

            while (
                node != INVALID_INDEX
            )
            {
                const OrderNode& current =
                    orderPool[node];

                remaining -=
                    current.order.quantity;

                if (remaining <= 0)
                {
                    return true;
                }

                node =
                    current.next;
            }
        }
    }


    // ========================================================
    // SELL
    // ========================================================

    else
    {
        for (
            const auto& [price, level] :
            bids
        )
        {
            // LIMIT SELL cannot consume
            // bids below its limit price.

            if (
                order.type == OrderType::LIMIT &&
                order.price > price
            )
            {
                break;
            }

            NodeIndex node =
                level.head;

            while (
                node != INVALID_INDEX
            )
            {
                const OrderNode& current =
                    orderPool[node];

                remaining -=
                    current.order.quantity;

                if (remaining <= 0)
                {
                    return true;
                }

                node =
                    current.next;
            }
        }
    }


    return false;
}


// ============================================================
// CHECK WHETHER ORDER WOULD IMMEDIATELY TRADE
// ============================================================

bool OrderBook::wouldImmediatelyTrade(
    const Order& order
) const
{
    // ========================================================
    // BUY
    // ========================================================

    if (order.side == Side::BUY)
    {
        if (asks.empty())
        {
            return false;
        }

        double bestAsk =
            asks.begin()->first;

        if (
            order.type ==
            OrderType::MARKET
        )
        {
            return true;
        }

        return order.price >= bestAsk;
    }


    // ========================================================
    // SELL
    // ========================================================

    if (bids.empty())
    {
        return false;
    }

    double bestBid =
        bids.begin()->first;

    if (
        order.type ==
        OrderType::MARKET
    )
    {
        return true;
    }

    return order.price <= bestBid;
}


// ============================================================
// MATCH ORDER
// ============================================================

void OrderBook::matchOrder(
    Order& incomingOrder
)
{
    // ========================================================
    // BUY
    // ========================================================

    if (incomingOrder.side == Side::BUY)
    {
        while (
            incomingOrder.quantity > 0 &&
            !asks.empty()
        )
        {
            auto bestAsk =
                asks.begin();

            double askPrice =
                bestAsk->first;

            // LIMIT BUY price protection

            if (
                incomingOrder.type == OrderType::LIMIT &&
                incomingOrder.price < askPrice
            )
            {
                break;
            }

            PriceLevel& level =
                bestAsk->second;

            NodeIndex node =
                level.head;

            OrderNode& sellNode =
                orderPool[node];

            int tradeQuantity =
                min(
                    incomingOrder.quantity,
                    sellNode.order.quantity
                );

            double tradePrice =
                sellNode.order.price;

            recordTrade(
                incomingOrder,
                sellNode.order,
                tradePrice,
                tradeQuantity
            );

            incomingOrder.quantity -=
                tradeQuantity;

            sellNode.order.quantity -=
                tradeQuantity;


            // ------------------------------------------------
            // PARTIALLY FILLED SELL
            // ------------------------------------------------

            if (
                sellNode.order.quantity > 0
            )
            {
                orderStatus[
                    sellNode.order.id
                ] =
                    OrderStatus::PARTIALLY_FILLED;
            }


            // ------------------------------------------------
            // FULLY FILLED SELL
            // ------------------------------------------------

            else
            {
                orderStatus[
                    sellNode.order.id
                ] =
                    OrderStatus::FILLED;

                orderIndex.erase(
                    sellNode.order.id
                );

                removeNodeFromPriceLevel(
                    level,
                    node
                );

                releaseNode(
                    node
                );
            }


            // ------------------------------------------------
            // REMOVE EMPTY PRICE LEVEL
            // ------------------------------------------------

            if (level.size == 0)
            {
                asks.erase(
                    bestAsk
                );
            }
        }
    }


    // ========================================================
    // SELL
    // ========================================================

    else
    {
        while (
            incomingOrder.quantity > 0 &&
            !bids.empty()
        )
        {
            auto bestBid =
                bids.begin();

            double bidPrice =
                bestBid->first;

            // LIMIT SELL price protection

            if (
                incomingOrder.type == OrderType::LIMIT &&
                incomingOrder.price > bidPrice
            )
            {
                break;
            }

            PriceLevel& level =
                bestBid->second;

            NodeIndex node =
                level.head;

            OrderNode& buyNode =
                orderPool[node];

            int tradeQuantity =
                min(
                    incomingOrder.quantity,
                    buyNode.order.quantity
                );

            double tradePrice =
                buyNode.order.price;

            recordTrade(
                buyNode.order,
                incomingOrder,
                tradePrice,
                tradeQuantity
            );

            incomingOrder.quantity -=
                tradeQuantity;

            buyNode.order.quantity -=
                tradeQuantity;


            // ------------------------------------------------
            // PARTIALLY FILLED BUY
            // ------------------------------------------------

            if (
                buyNode.order.quantity > 0
            )
            {
                orderStatus[
                    buyNode.order.id
                ] =
                    OrderStatus::PARTIALLY_FILLED;
            }


            // ------------------------------------------------
            // FULLY FILLED BUY
            // ------------------------------------------------

            else
            {
                orderStatus[
                    buyNode.order.id
                ] =
                    OrderStatus::FILLED;

                orderIndex.erase(
                    buyNode.order.id
                );

                removeNodeFromPriceLevel(
                    level,
                    node
                );

                releaseNode(
                    node
                );
            }


            // ------------------------------------------------
            // REMOVE EMPTY PRICE LEVEL
            // ------------------------------------------------

            if (level.size == 0)
            {
                bids.erase(
                    bestBid
                );
            }
        }
    }
}


// ============================================================
// CANCEL REMAINING ORDER
// ============================================================

void OrderBook::cancelRemainingOrder(
    int orderId
)
{
    auto it =
        orderIndex.find(orderId);

    if (
        it != orderIndex.end()
    )
    {
        cancelOrder(orderId);
    }
    else
    {
        orderStatus[orderId] =
            OrderStatus::CANCELLED;
    }
}


// ============================================================
// ADD ORDER
// ============================================================

bool OrderBook::addOrder(
    Order order
)
{
    // --------------------------------------------------------
    // VALIDATION
    // --------------------------------------------------------

    if (!validateOrder(order))
    {
        if (
            order.id > 0 &&
            orderIndex.find(order.id)
                == orderIndex.end()
        )
        {
            orderStatus[order.id] =
                OrderStatus::REJECTED;
        }

        return false;
    }


    // --------------------------------------------------------
    // POST ONLY
    // --------------------------------------------------------

    if (
        order.postOnly &&
        wouldImmediatelyTrade(order)
    )
    {
        orderStatus[order.id] =
            OrderStatus::REJECTED;

#ifndef ORDERBOOK_SILENT

        cout << "\n===== POST-ONLY REJECTED =====\n";

        cout << "Order ID: "
             << order.id
             << "\n";

        cout << "Order would immediately execute.\n";

#endif

        return false;
    }


    // --------------------------------------------------------
    // FOK
    // --------------------------------------------------------

    if (
        order.timeInForce ==
        TimeInForce::FOK
    )
    {
        if (
            !canFullyFill(order)
        )
        {
            orderStatus[order.id] =
                OrderStatus::CANCELLED;

#ifndef ORDERBOOK_SILENT

            cout << "\n===== FOK CANCELLED =====\n";

            cout << "Order ID: "
                 << order.id
                 << "\n";

            cout
                << "Full quantity is not available.\n";

#endif

            return true;
        }
    }


    // --------------------------------------------------------
    // INITIAL STATUS
    // --------------------------------------------------------

    orderStatus[order.id] =
        OrderStatus::OPEN;

    int originalQuantity =
        order.quantity;


    // --------------------------------------------------------
    // MATCH
    // --------------------------------------------------------

    matchOrder(order);


    // --------------------------------------------------------
    // FULLY FILLED
    // --------------------------------------------------------

    if (
        order.quantity == 0
    )
    {
        orderStatus[order.id] =
            OrderStatus::FILLED;

        return true;
    }


    // --------------------------------------------------------
    // PARTIAL EXECUTION
    // --------------------------------------------------------

    bool wasPartiallyFilled =
        order.quantity <
        originalQuantity;

    if (wasPartiallyFilled)
    {
        orderStatus[order.id] =
            OrderStatus::PARTIALLY_FILLED;
    }


    // ========================================================
    // IOC
    // ========================================================

    if (
        order.timeInForce ==
        TimeInForce::IOC
    )
    {
        /*
         * IOC never rests in the book.
         *
         * Whether partially filled or not,
         * the remaining quantity is cancelled.
         *
         * Therefore final status is CANCELLED.
         */

        orderStatus[order.id] =
            OrderStatus::CANCELLED;

#ifndef ORDERBOOK_SILENT

        cout
            << "\n===== IOC REMAINDER CANCELLED =====\n";

        cout << "Order ID: "
             << order.id
             << "\n";

        cout << "Remaining Quantity: "
             << order.quantity
             << "\n";

#endif

        return true;
    }


    // ========================================================
    // MARKET ORDER
    // ========================================================

    if (
        order.type ==
        OrderType::MARKET
    )
    {
        /*
         * IMPORTANT FIX
         *
         * Case 1:
         *
         * Market order executes nothing.
         *
         * originalQuantity == remaining quantity
         *
         * Final status:
         * CANCELLED
         *
         *
         * Case 2:
         *
         * Market order executes partially.
         *
         * remaining quantity < original quantity
         *
         * Final status:
         * PARTIALLY_FILLED
         *
         *
         * Case 3:
         *
         * Market order executes completely.
         *
         * Already handled above as FILLED.
         */

        if (
            wasPartiallyFilled
        )
        {
            orderStatus[order.id] =
                OrderStatus::PARTIALLY_FILLED;
        }
        else
        {
            orderStatus[order.id] =
                OrderStatus::CANCELLED;
        }


#ifndef ORDERBOOK_SILENT

        cout
            << "\n===== MARKET ORDER REMAINDER CANCELLED =====\n";

        cout << "Order ID: "
             << order.id
             << "\n";

        cout << "Remaining Quantity: "
             << order.quantity
             << "\n";

#endif

        return true;
    }


    // ========================================================
    // LIMIT BUY
    // ========================================================

    if (
        order.side ==
        Side::BUY
    )
    {
        auto [iterator, inserted] =
            bids.try_emplace(
                order.price
            );

        PriceLevel& level =
            iterator->second;

        NodeIndex node =
            createNode(order);

        addNodeToPriceLevel(
            level,
            node
        );

        orderIndex.emplace(
            order.id,
            node
        );
    }


    // ========================================================
    // LIMIT SELL
    // ========================================================

    else
    {
        auto [iterator, inserted] =
            asks.try_emplace(
                order.price
            );

        PriceLevel& level =
            iterator->second;

        NodeIndex node =
            createNode(order);

        addNodeToPriceLevel(
            level,
            node
        );

        orderIndex.emplace(
            order.id,
            node
        );
    }


    return true;
}


// ============================================================
// CANCEL ORDER
// ============================================================

bool OrderBook::cancelOrder(
    int orderId
)
{
    auto indexIt =
        orderIndex.find(orderId);

    if (
        indexIt ==
        orderIndex.end()
    )
    {
#ifndef ORDERBOOK_SILENT

        cout
            << "\n===== CANCELLATION FAILED =====\n";

        cout << "Order ID: "
             << orderId
             << " not found.\n";

#endif

        return false;
    }

    NodeIndex node =
        indexIt->second;

    OrderNode& orderNode =
        orderPool[node];

    Side side =
        orderNode.order.side;

    double price =
        orderNode.order.price;


    // ========================================================
    // BUY
    // ========================================================

    if (
        side == Side::BUY
    )
    {
        auto priceLevel =
            bids.find(price);

        if (
            priceLevel ==
            bids.end()
        )
        {
            return false;
        }

        PriceLevel& level =
            priceLevel->second;

        removeNodeFromPriceLevel(
            level,
            node
        );

        if (
            level.size == 0
        )
        {
            bids.erase(
                priceLevel
            );
        }
    }


    // ========================================================
    // SELL
    // ========================================================

    else
    {
        auto priceLevel =
            asks.find(price);

        if (
            priceLevel ==
            asks.end()
        )
        {
            return false;
        }

        PriceLevel& level =
            priceLevel->second;

        removeNodeFromPriceLevel(
            level,
            node
        );

        if (
            level.size == 0
        )
        {
            asks.erase(
                priceLevel
            );
        }
    }


    // --------------------------------------------------------
    // REMOVE INDEX
    // --------------------------------------------------------

    orderIndex.erase(
        indexIt
    );


    // --------------------------------------------------------
    // STATUS
    // --------------------------------------------------------

    orderStatus[orderId] =
        OrderStatus::CANCELLED;


    // --------------------------------------------------------
    // RELEASE NODE
    // --------------------------------------------------------

    releaseNode(
        node
    );


#ifndef ORDERBOOK_SILENT

    cout << "\n===== ORDER CANCELLED =====\n";

    cout << "Order ID: "
         << orderId
         << "\n";

#endif


    return true;
}


// ============================================================
// MODIFY ORDER
// ============================================================

bool OrderBook::modifyOrder(
    int orderId,
    double newPrice,
    int newQuantity
)
{
    // --------------------------------------------------------
    // VALIDATE QUANTITY
    // --------------------------------------------------------

    if (
        newQuantity <= 0
    )
    {
#ifndef ORDERBOOK_SILENT

        cout
            << "\n===== MODIFICATION FAILED =====\n";

        cout
            << "Quantity must be greater than 0.\n";

#endif

        return false;
    }


    // --------------------------------------------------------
    // VALIDATE PRICE
    // --------------------------------------------------------

    if (
        newPrice <= 0
    )
    {
#ifndef ORDERBOOK_SILENT

        cout
            << "\n===== MODIFICATION FAILED =====\n";

        cout
            << "Price must be greater than 0.\n";

#endif

        return false;
    }


    // --------------------------------------------------------
    // FIND ORDER
    // --------------------------------------------------------

    auto indexIt =
        orderIndex.find(orderId);

    if (
        indexIt ==
        orderIndex.end()
    )
    {
#ifndef ORDERBOOK_SILENT

        cout
            << "\n===== MODIFICATION FAILED =====\n";

        cout << "Order ID: "
             << orderId
             << " not found.\n";

#endif

        return false;
    }


    NodeIndex node =
        indexIt->second;

    Order oldOrder =
        orderPool[node].order;

    Side side =
        oldOrder.side;

    double oldPrice =
        oldOrder.price;


    // --------------------------------------------------------
    // CANCEL OLD ORDER
    // --------------------------------------------------------

    if (
        !cancelOrder(orderId)
    )
    {
        return false;
    }


    // --------------------------------------------------------
    // CREATE MODIFIED ORDER
    // --------------------------------------------------------

    Order modifiedOrder;

    modifiedOrder.id =
        oldOrder.id;

    modifiedOrder.side =
        side;

    modifiedOrder.type =
        OrderType::LIMIT;

    modifiedOrder.price =
        newPrice;

    modifiedOrder.quantity =
        newQuantity;

    // Modification loses time priority.

    modifiedOrder.timestamp =
        ++currentTimestamp;

    modifiedOrder.timeInForce =
        TimeInForce::GTC;

    modifiedOrder.postOnly =
        false;


    // --------------------------------------------------------
    // ADD MODIFIED ORDER
    // --------------------------------------------------------

    bool success =
        addOrder(
            modifiedOrder
        );

    if (!success)
    {
        return false;
    }


#ifndef ORDERBOOK_SILENT

    cout
        << "\n===== ORDER MODIFIED =====\n";

    cout << "Order ID: "
         << orderId
         << "\n";

    cout << "Old Price: "
         << oldPrice
         << "\n";

    cout << "New Price: "
         << newPrice
         << "\n";

    cout << "New Quantity: "
         << newQuantity
         << "\n";

    cout << "New Timestamp: "
         << modifiedOrder.timestamp
         << "\n";

#endif


    return true;
}


// ============================================================
// PRINT BOOK
// ============================================================

void OrderBook::printBook() const
{
    cout
        << "\n========================================\n";

    cout
        << "              ORDER BOOK\n";

    cout
        << "========================================\n";


    // ========================================================
    // SELL ORDERS
    // ========================================================

    cout
        << "\n========== SELL ORDERS ==========\n";

    if (
        asks.empty()
    )
    {
        cout
            << "No SELL orders.\n";
    }
    else
    {
        for (
            const auto& [price, level] :
            asks
        )
        {
            cout
                << "Price: "
                << price
                << " | Orders: "
                << level.size
                << "\n";

            NodeIndex node =
                level.head;

            while (
                node != INVALID_INDEX
            )
            {
                const OrderNode& current =
                    orderPool[node];

                cout
                    << "    Order ID: "
                    << current.order.id
                    << " | Quantity: "
                    << current.order.quantity
                    << " | Timestamp: "
                    << current.order.timestamp
                    << "\n";

                node =
                    current.next;
            }
        }
    }


    // ========================================================
    // BUY ORDERS
    // ========================================================

    cout
        << "\n========== BUY ORDERS ==========\n";

    if (
        bids.empty()
    )
    {
        cout
            << "No BUY orders.\n";
    }
    else
    {
        for (
            const auto& [price, level] :
            bids
        )
        {
            cout
                << "Price: "
                << price
                << " | Orders: "
                << level.size
                << "\n";

            NodeIndex node =
                level.head;

            while (
                node != INVALID_INDEX
            )
            {
                const OrderNode& current =
                    orderPool[node];

                cout
                    << "    Order ID: "
                    << current.order.id
                    << " | Quantity: "
                    << current.order.quantity
                    << " | Timestamp: "
                    << current.order.timestamp
                    << "\n";

                node =
                    current.next;
            }
        }
    }
}


// ============================================================
// PRINT TRADE HISTORY
// ============================================================

void OrderBook::printTradeHistory() const
{
    cout
        << "\n========================================\n";

    cout
        << "             TRADE HISTORY\n";

    cout
        << "========================================\n";


    if (
        tradeHistory.empty()
    )
    {
        cout
            << "No trades executed.\n";

        return;
    }


    for (
        const auto& trade :
        tradeHistory
    )
    {
        cout << "\n";

        cout
            << "Buy Order ID: "
            << trade.buyOrderId
            << "\n";

        cout
            << "Sell Order ID: "
            << trade.sellOrderId
            << "\n";

        cout
            << "Trade Price: "
            << trade.price
            << "\n";

        cout
            << "Trade Quantity: "
            << trade.quantity
            << "\n";

        cout
            << "Trade Timestamp: "
            << trade.timestamp
            << "\n";
    }
}


// ============================================================
// BEST BID
// ============================================================

double OrderBook::getBestBid() const
{
    if (
        bids.empty()
    )
    {
        return -1;
    }

    return bids.begin()->first;
}


// ============================================================
// BEST ASK
// ============================================================

double OrderBook::getBestAsk() const
{
    if (
        asks.empty()
    )
    {
        return -1;
    }

    return asks.begin()->first;
}


// ============================================================
// GET ORDER QUANTITY
// ============================================================

int OrderBook::getOrderQuantity(
    int orderId
) const
{
    // --------------------------------------------------------
    // 1. Check whether the order is currently active.
    // --------------------------------------------------------
    auto iterator =
        orderIndex.find(orderId);

    if (
        iterator !=
        orderIndex.end()
    )
    {
        NodeIndex node =
            iterator->second;

        return orderPool[node]
            .order
            .quantity;
    }

    // --------------------------------------------------------
    // 2. Order is no longer active.
    //    Check whether we have a stored status for it.
    // --------------------------------------------------------
    auto statusIterator =
        orderStatus.find(orderId);

    if (
        statusIterator !=
        orderStatus.end()
    )
    {
        // ----------------------------------------------------
        // Cancelled order:
        // The order has been removed from the book, so its
        // remaining quantity is zero.
        // ----------------------------------------------------
        if (
            statusIterator->second ==
            OrderStatus::CANCELLED
        )
        {
            return 0;
        }

        // ----------------------------------------------------
        // Fully filled order:
        // There is no remaining quantity.
        // ----------------------------------------------------
        if (
            statusIterator->second ==
            OrderStatus::FILLED
        )
        {
            return 0;
        }
    }

    // --------------------------------------------------------
    // 3. Order does not exist.
    // --------------------------------------------------------
    return -1;
}


// ============================================================
// GET TRADE COUNT
// ============================================================

int OrderBook::getTradeCount() const
{
    return static_cast<int>(
        tradeHistory.size()
    );
}


// ============================================================
// GET ORDER STATUS
// ============================================================

OrderStatus OrderBook::getOrderStatus(
    int orderId
) const
{
    auto iterator =
        orderStatus.find(orderId);

    if (
        iterator ==
        orderStatus.end()
    )
    {
        return OrderStatus::UNKNOWN;
    }

    return iterator->second;
}


// ============================================================
// GET TRADE HISTORY
// ============================================================

const std::vector<Trade>&
OrderBook::getTradeHistory() const
{
    return tradeHistory;
}