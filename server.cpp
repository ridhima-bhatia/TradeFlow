#include <iostream>
#include <sstream>
#include <chrono>
#include <atomic>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sys/select.h>

#include "OrderBook.h"
#include "ThreadPool.h"
#include "TradeLogger.h"
#include "Config.h"

using namespace std;

static atomic<bool> keepRunning{true};

// Ctrl+C handler: just flip a flag. The accept loop below polls this flag
// via select()'s timeout instead of relying on shutdown()/close() to
// interrupt a blocking accept() call -- that trick is not portable (it
// reliably unblocks accept() on Linux but often does NOT on macOS/BSD).
// Polling with select() works identically on both.
void signalHandler(int)
{
    keepRunning = false;
}

void handleClient(int clientSocket, OrderBook& book, TradeLogger& logger)
{
    char buffer[1024] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived <= 0)
    {
        close(clientSocket);
        return;
    }
    buffer[bytesReceived] = '\0';

    stringstream ss(buffer);
    char side;
    int id, price, quantity;
    ss >> side >> id >> price >> quantity;

    // Basic validation -- reject garbage instead of silently misbehaving.
    if ((side != 'B' && side != 'S') || price <= 0 || quantity <= 0)
    {
        string reject = "REJECT invalid order format or non-positive price/qty";
        send(clientSocket, reject.c_str(), reject.length(), 0);
        logger.log("REJECT raw=\"" + string(buffer) + "\"");
        close(clientSocket);
        return;
    }

    Side orderSide = (side == 'B') ? Side::BUY : Side::SELL;
    Order order(id, orderSide, price, quantity);

    auto start = chrono::steady_clock::now();
    OrderResult result = book.addOrder(order);
    auto end = chrono::steady_clock::now();

    long long micros = chrono::duration_cast<chrono::microseconds>(end - start).count();

    ostringstream ack;
    ack << "ACK order#" << result.orderId
        << " filled=" << result.filledQty
        << " remaining=" << result.remainingQty
        << " latency_us=" << micros
        << " " << result.trades;

    string ackStr = ack.str();
    send(clientSocket, ackStr.c_str(), ackStr.length(), 0);

    // Persist to disk -- an audit trail, separate from the in-memory book.
    logger.log(ackStr);

    close(clientSocket);
}

int main()
{
    Config config("server.conf");
    int port = config.getInt("port", 8080);
    int poolSize = config.getInt("thread_pool_size", 8);
    string logFile = config.getString("log_file", "trades.log");

    signal(SIGINT, signalHandler);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) { cout << "Socket creation failed!\n"; return 1; }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (::bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
    { cout << "Bind failed!\n"; return 1; }

    if (listen(serverSocket, SOMAXCONN) == -1)
    { cout << "Listen failed!\n"; return 1; }

    OrderBook book;
    TradeLogger logger(logFile);
    ThreadPool pool(poolSize); // fixed N worker threads, created once up front

    cout << "Server listening on port " << port
         << " with a pool of " << poolSize << " worker threads.\n"
         << "Trade log: " << logFile << "  (Ctrl+C to stop)\n";
    cout.flush();

    while (keepRunning)
    {
        // Wait up to 1 second for an incoming connection. This bounds how
        // long a blocking accept() call could otherwise sit idle, so the
        // loop wakes up regularly to re-check keepRunning -- portable across
        // Linux and macOS, unlike relying on shutdown()/close() to interrupt
        // a blocked accept().
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        struct timeval tv{1, 0}; // 1 second

        int ready = select(serverSocket + 1, &readfds, nullptr, nullptr, &tv);

        if (ready <= 0)
            continue; // timeout (no connection yet) or interrupted -- loop back and check keepRunning

        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == -1)
            continue;

        // Hand the socket to the pool instead of spawning a new OS thread.
        // A fixed number of workers process an unbounded number of clients.
        pool.submit([clientSocket, &book, &logger]() {
            handleClient(clientSocket, book, logger);
        });
    }

    close(serverSocket);
    cout << "\nShutting down: draining remaining queued orders...\n";
    // ThreadPool's destructor runs here (pool goes out of scope), which
    // notifies all workers and joins them after they finish queued work.
    return 0;
}