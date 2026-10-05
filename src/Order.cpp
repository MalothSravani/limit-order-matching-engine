#include<bits/stdc++.h>
using namespace std;
enum class Side {
    BUY,
    SELL
};
enum class OrderType {
    LIMIT,
    MARKET
};
struct Order {
    int id;
    Side side;
    OrderType type;
    double price;
    int quantity;
    long long timestamp;
};