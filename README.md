# Stock Exchange Simulator

A TCP client-server stock exchange simulator in C++ implementing a limit order book with price-time priority matching, concurrent order processing, and persistent trade logging.

## Build
```
g++ -std=c++17 -pthread -c OrderBook.cpp -o OrderBook.o
g++ -std=c++17 -pthread server.cpp OrderBook.o -o server
g++ -std=c++17 -pthread client.cpp -o client
g++ -std=c++17 -pthread bench_client.cpp -o bench_client
```

## Run
```
./server              # reads port/pool size/log path from server.conf
./client               # interactive single-order client
./bench_client 8 500   # load test: 8 threads x 500 orders each
```
Stop the server with Ctrl+C — it stops accepting new connections and finishes queued work before exiting.

## Architecture
- **ThreadPool.h** — fixed-size pool of worker threads pulling tasks off a shared queue, synchronized with a mutex and condition_variable. The number of OS threads stays constant regardless of how many clients connect.
- **OrderBook** — mutex-guarded price-time priority matching engine (a map of price levels to a FIFO queue of orders on each side).
- **TradeLogger.h** — thread-safe, append-only audit log (`trades.log`); every processed order is logged with a timestamp, fill status, and processing latency.
- **Config.h** — reads server settings (port, thread pool size, log file path) from `server.conf`, with sane defaults if the file is missing.
- **bench_client.cpp** — concurrent load generator that fires orders from multiple client threads simultaneously and reports throughput and latency percentiles (p50/p95/p99).

## Benchmark
Run `./bench_client <num_threads> <orders_per_thread>` against a running server to measure throughput and latency on your own machine. Example output format:
```
Total orders completed : 400
Throughput              : 9895.66 orders/sec
Latency (ms): min=0.06  avg=0.68  p50=0.60  p95=1.46  p99=1.99  max=2.94
```

## Design notes
- The order book uses a single coarse-grained lock rather than per-price-level locking. This is the simpler, correct starting point; sharding the lock by price level would reduce contention under heavier concurrent load.
- Concurrency is handled via a fixed-size thread pool rather than thread-per-connection, so thread count doesn't grow with client count.

## Future work
- Order cancellation by ID
- Event-driven I/O (epoll/io_uring) as an alternative to the thread pool for scaling beyond low-thousands of concurrent connections
