#ifndef ORDER_H
#define ORDER_H

// ============================================================
// SIDE
// ============================================================

enum class Side {
    BUY,
    SELL
};


// ============================================================
// ORDER TYPE
// ============================================================

enum class OrderType {
    LIMIT,
    MARKET
};


// ============================================================
// TIME IN FORCE
// ============================================================

enum class TimeInForce {

    // Normal order.
    // Unfilled quantity remains in the book.
    GTC,

    // Execute immediately.
    // Any remaining quantity is cancelled.
    IOC,

    // Execute the entire quantity or cancel everything.
    FOK
};


// ============================================================
// ORDER STATUS
// ============================================================

enum class OrderStatus {

    OPEN,

    PARTIALLY_FILLED,

    FILLED,

    CANCELLED,

    REJECTED,

    UNKNOWN
};


// ============================================================
// ORDER
// ============================================================

struct Order {

    int id;

    Side side;

    OrderType type;

    double price;

    int quantity;

    long long timestamp;

    // New fields.
    //
    // Defaults keep your old 6-field order creation valid.

    TimeInForce timeInForce = TimeInForce::GTC;

    bool postOnly = false;
};

#endif