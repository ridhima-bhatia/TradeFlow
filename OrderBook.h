#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <map>
#include <queue>
#include <mutex>
#include <string>
#include "Order.h"

// Result of processing one incoming order — returned so the server can
// send a meaningful ack back to the client instead of nothing at all.
struct OrderResult
{
    int orderId;
    int filledQty;
    int remainingQty;
    std::string trades; // human-readable summary of each fill
};

class OrderBook
{
private:
    std::map<int, std::queue<Order>, std::greater<int>> buyBook;
    std::map<int, std::queue<Order>> sellBook;

    // Single mutex guarding both books. A real low-latency system would
    // shard this per price level or use a lock-free structure, but a
    // single coarse-grained lock is the correct first step and is what
    // interviewers expect you to be able to explain the tradeoffs of.
    mutable std::mutex bookMutex;

    OrderResult matchBuy(Order& buyOrder);
    OrderResult matchSell(Order& sellOrder);

public:
    // Thread-safe: multiple client-handling threads can call this concurrently.
    OrderResult addOrder(const Order& incoming);

    void printOrderBook() const;
};

#endif
