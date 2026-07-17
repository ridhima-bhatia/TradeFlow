#include "OrderBook.h"
#include <iostream>
#include <algorithm>
#include <sstream>

using namespace std;

// ============================================
// Add Order (thread-safe entry point)
// ============================================
OrderResult OrderBook::addOrder(const Order& incoming)
{
    Order order = incoming;

    lock_guard<mutex> lock(bookMutex); // scoped lock: released automatically on return

    if (order.side == Side::BUY)
        return matchBuy(order);
    else
        return matchSell(order);
}

// ============================================
// Print Order Book
// ============================================
void OrderBook::printOrderBook() const
{
    lock_guard<mutex> lock(bookMutex);

    cout << "\n===== ORDER BOOK =====\n";

    cout << "\nBUY SIDE (Price Desc):\n";
    for (const auto& [price, q] : buyBook)
        cout << "Price " << price << " | Orders " << q.size() << endl;

    cout << "\nSELL SIDE (Price Asc):\n";
    for (const auto& [price, q] : sellBook)
        cout << "Price " << price << " | Orders " << q.size() << endl;
}

// ============================================
// Match Buy Orders
// ============================================
OrderResult OrderBook::matchBuy(Order& buyOrder)
{
    OrderResult result{buyOrder.id, 0, 0, ""};
    ostringstream trades;

    while (buyOrder.quantity > 0 && !sellBook.empty())
    {
        auto bestSell = sellBook.begin();

        if (bestSell->first > buyOrder.price)
            break;

        auto& sellQueue = bestSell->second;

        while (!sellQueue.empty() && buyOrder.quantity > 0)
        {
            Order& sellOrder = sellQueue.front();

            int tradedQty = min(buyOrder.quantity, sellOrder.quantity);

            buyOrder.quantity -= tradedQty;
            sellOrder.quantity -= tradedQty;
            result.filledQty += tradedQty;

            trades << "[FILL " << tradedQty << "@" << bestSell->first
                   << " vs sell#" << sellOrder.id << "] ";

            if (sellOrder.quantity == 0)
                sellQueue.pop();
        }

        if (sellQueue.empty())
            sellBook.erase(bestSell);
    }

    if (buyOrder.quantity > 0)
        buyBook[buyOrder.price].push(buyOrder);

    result.remainingQty = buyOrder.quantity;
    result.trades = trades.str();
    return result;
}

// ============================================
// Match Sell Orders
// ============================================
OrderResult OrderBook::matchSell(Order& sellOrder)
{
    OrderResult result{sellOrder.id, 0, 0, ""};
    ostringstream trades;

    while (sellOrder.quantity > 0 && !buyBook.empty())
    {
        auto bestBuy = buyBook.begin();

        if (bestBuy->first < sellOrder.price)
            break;

        auto& buyQueue = bestBuy->second;

        while (!buyQueue.empty() && sellOrder.quantity > 0)
        {
            Order& buyOrder = buyQueue.front();

            int tradedQty = min(sellOrder.quantity, buyOrder.quantity);

            sellOrder.quantity -= tradedQty;
            buyOrder.quantity -= tradedQty;
            result.filledQty += tradedQty;

            trades << "[FILL " << tradedQty << "@" << bestBuy->first
                   << " vs buy#" << buyOrder.id << "] ";

            if (buyOrder.quantity == 0)
                buyQueue.pop();
        }

        if (buyQueue.empty())
            buyBook.erase(bestBuy);
    }

    if (sellOrder.quantity > 0)
        sellBook[sellOrder.price].push(sellOrder);

    result.remainingQty = sellOrder.quantity;
    result.trades = trades.str();
    return result;
}
