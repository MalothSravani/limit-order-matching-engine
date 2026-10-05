#ifndef TRADE_H
#define TRADE_H

struct Trade {

    int buyOrderId;

    int sellOrderId;

    double price;

    int quantity;

    long long timestamp;
};

#endif