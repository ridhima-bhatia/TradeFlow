// Concurrent load generator for the Stock Exchange Simulator.
//
// Fires ORDERS_PER_THREAD orders from each of NUM_THREADS client threads
// at the server simultaneously, measures round-trip latency for every
// order, and reports throughput + latency percentiles at the end.
//
// This is what gives you a REAL, defensible number for your resume
// instead of a made-up one, e.g.:
//   "Benchmarked 8 concurrent clients issuing 5,000 orders, achieving
//    X orders/sec with p95 latency of Y ms"
//
// Build:  g++ -std=c++17 -pthread bench_client.cpp -o bench_client
// Run:    ./bench_client [num_threads] [orders_per_thread]

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <chrono>
#include <random>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;
using namespace chrono;

static mutex resultsMutex;
static vector<double> latenciesMs; // shared results, guarded by resultsMutex

void worker(int threadId, int ordersPerThread, int basePrice)
{
    mt19937 rng(threadId * 7919 + 1);
    uniform_int_distribution<int> priceJitter(-5, 5);
    uniform_int_distribution<int> qtyDist(1, 100);

    vector<double> localLatencies;
    localLatencies.reserve(ordersPerThread);

    for (int i = 0; i < ordersPerThread; ++i)
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) continue;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(8080);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        auto t0 = steady_clock::now();

        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == -1)
        {
            close(sock);
            continue;
        }

        char side = (i % 2 == 0) ? 'B' : 'S';
        int id = threadId * 100000 + i;
        int price = basePrice + priceJitter(rng);
        int qty = qtyDist(rng);

        string msg = string(1, side) + " " + to_string(id) + " " +
                     to_string(price) + " " + to_string(qty);

        send(sock, msg.c_str(), msg.length(), 0);

        char buf[1024] = {0};
        recv(sock, buf, sizeof(buf) - 1, 0);

        auto t1 = steady_clock::now();
        double ms = duration_cast<duration<double, milli>>(t1 - t0).count();
        localLatencies.push_back(ms);

        close(sock);
    }

    lock_guard<mutex> lock(resultsMutex);
    latenciesMs.insert(latenciesMs.end(), localLatencies.begin(), localLatencies.end());
}

int main(int argc, char* argv[])
{
    int numThreads = (argc > 1) ? stoi(argv[1]) : 8;
    int ordersPerThread = (argc > 2) ? stoi(argv[2]) : 500;

    cout << "Launching " << numThreads << " concurrent client threads, "
         << ordersPerThread << " orders each ("
         << numThreads * ordersPerThread << " total orders)...\n";

    auto benchStart = steady_clock::now();

    vector<thread> threads;
    for (int t = 0; t < numThreads; ++t)
        threads.emplace_back(worker, t, ordersPerThread, 100);

    for (auto& th : threads) th.join();

    auto benchEnd = steady_clock::now();
    double totalSeconds = duration_cast<duration<double>>(benchEnd - benchStart).count();

    sort(latenciesMs.begin(), latenciesMs.end());
    int n = latenciesMs.size();

    if (n == 0)
    {
        cout << "No successful orders — is the server running?\n";
        return 1;
    }

    double sum = 0;
    for (double v : latenciesMs) sum += v;

    double avg = sum / n;
    double p50 = latenciesMs[n * 50 / 100];
    double p95 = latenciesMs[min(n - 1, n * 95 / 100)];
    double p99 = latenciesMs[min(n - 1, n * 99 / 100)];
    double minL = latenciesMs.front();
    double maxL = latenciesMs.back();
    double throughput = n / totalSeconds;

    cout << "\n===== BENCHMARK RESULTS =====\n";
    cout << "Total orders completed : " << n << "\n";
    cout << "Wall clock time         : " << totalSeconds << " s\n";
    cout << "Throughput               : " << throughput << " orders/sec\n";
    cout << "Latency (ms)  min=" << minL << " avg=" << avg
         << " p50=" << p50 << " p95=" << p95
         << " p99=" << p99 << " max=" << maxL << "\n";

    return 0;
}
