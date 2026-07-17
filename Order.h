#ifndef ORDER_H
#define ORDER_H

#include <chrono>

enum class Side
{
    BUY,
    SELL
};

struct Order
{
    int id;
    Side side;
    int price;
    int quantity;
    std::chrono::steady_clock::time_point receivedAt; // used for server-side latency measurement

    Order(int id, Side side, int price, int quantity)
        : id(id), side(side), price(price), quantity(quantity),
          receivedAt(std::chrono::steady_clock::now()) {}
};

#endif
