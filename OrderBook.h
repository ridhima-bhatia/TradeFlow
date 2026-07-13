#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <map>
#include <queue>
#include "Order.h"

using namespace std;

class OrderBook
{
private:
    map<int, queue<Order>, greater<int>> buyBook;
    map<int, queue<Order>> sellBook;

    void matchBuy(Order& buyOrder);
    void matchSell(Order& sellOrder);

public:
    void addOrder(const Order& incoming);

    void printOrderBook() const;
};

#endif