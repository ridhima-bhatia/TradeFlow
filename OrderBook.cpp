#include "OrderBook.h"
#include <iostream>
#include <algorithm>

using namespace std;

// ============================================
// Add Order
// ============================================
void OrderBook::addOrder(const Order& incoming)
{
    Order order = incoming;

    if (order.side == Side::BUY)
        matchBuy(order);
    else
        matchSell(order);
}

// ============================================
// Print Order Book
// ============================================
void OrderBook::printOrderBook() const
{
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
void OrderBook::matchBuy(Order& buyOrder)
{
    while (buyOrder.quantity > 0 && !sellBook.empty())
    {
        auto bestSell = sellBook.begin();

        if (bestSell->first > buyOrder.price)
            break;

        auto& sellQueue = bestSell->second;

        while (!sellQueue.empty() && buyOrder.quantity > 0)
        {
            Order& sellOrder = sellQueue.front();

            int tradedQty = min(buyOrder.quantity,
                                sellOrder.quantity);

            buyOrder.quantity -= tradedQty;
            sellOrder.quantity -= tradedQty;

            cout << "\nTRADE EXECUTED\n";
            cout << "Buy ID  : " << buyOrder.id << endl;
            cout << "Sell ID : " << sellOrder.id << endl;
            cout << "Price   : " << bestSell->first << endl;
            cout << "Qty     : " << tradedQty << endl;

            if (sellOrder.quantity == 0)
                sellQueue.pop();
        }

        if (sellQueue.empty())
            sellBook.erase(bestSell);
    }

    if (buyOrder.quantity > 0)
        buyBook[buyOrder.price].push(buyOrder);
}

// ============================================
// Match Sell Orders
// ============================================
void OrderBook::matchSell(Order& sellOrder)
{
    while (sellOrder.quantity > 0 && !buyBook.empty())
    {
        auto bestBuy = buyBook.begin();

        if (bestBuy->first < sellOrder.price)
            break;

        auto& buyQueue = bestBuy->second;

        while (!buyQueue.empty() && sellOrder.quantity > 0)
        {
            Order& buyOrder = buyQueue.front();

            int tradedQty = min(sellOrder.quantity,
                                buyOrder.quantity);

            sellOrder.quantity -= tradedQty;
            buyOrder.quantity -= tradedQty;

            cout << "\nTRADE EXECUTED\n";
            cout << "Buy ID  : " << buyOrder.id << endl;
            cout << "Sell ID : " << sellOrder.id << endl;
            cout << "Price   : " << bestBuy->first << endl;
            cout << "Qty     : " << tradedQty << endl;

            if (buyOrder.quantity == 0)
                buyQueue.pop();
        }

        if (buyQueue.empty())
            buyBook.erase(bestBuy);
    }

    if (sellOrder.quantity > 0)
        sellBook[sellOrder.price].push(sellOrder);
}