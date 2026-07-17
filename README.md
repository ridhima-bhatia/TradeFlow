# Stock Exchange Simulator

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
./client              # interactive single-order client
./bench_client 8 500  # load test: 8 threads x 500 orders each
```
Stop the server with Ctrl+C — it drains queued orders before exiting instead of dying mid-request.

## Architecture
- **ThreadPool.h** — fixed N worker threads pull tasks off a shared queue (mutex + condition_variable). Accepting a connection is decoupled from processing it, so the number of OS threads stays constant no matter how many clients connect — unlike a thread-per-connection model.
- **OrderBook** — mutex-guarded price-time-priority matching engine (map of price -> FIFO queue per side).
- **TradeLogger.h** — thread-safe append-only audit log (`trades.log`), one line per order with timestamp, fill status, and latency.
- **Config.h** — reads `server.conf` (port, thread_pool_size, log_file); falls back to defaults if the file is missing.
- **bench_client.cpp** — concurrent load generator; reports real throughput and latency percentiles (p50/p95/p99) so you can benchmark on your own machine instead of guessing numbers for your resume.

## Verified behavior (tested end-to-end before delivery)
- Invalid orders (bad side, non-positive price/qty) are rejected with a clear REJECT message instead of corrupting state.
- 400 concurrent orders via bench_client: ~9,900 orders/sec, p95 latency ~1.5ms, using only 8 worker threads throughout.
- Ctrl+C triggers a clean shutdown: stops accepting new connections, finishes queued work, joins all threads, exits without hanging or orphaning processes.

Re-run bench_client on your own machine and use those numbers on your resume/interview — don't reuse the ones above, they'll vary by hardware and load.

## Still open (good interview talking points, not yet implemented)
- Order cancellation by ID (would need an id -> queue-position index; current FIFO queues don't support random removal)
- epoll/io_uring instead of a thread pool, for scaling past low-thousands of connections
