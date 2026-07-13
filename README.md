# TradeFlow – TCP Stock Exchange Simulator

TradeFlow is a TCP client-server based stock exchange simulator developed in C++. It demonstrates how a basic electronic exchange processes buy and sell orders using a limit order book and price-time priority matching.

## Features

- TCP client-server communication using POSIX sockets
- Limit order book implementation
- Price-time priority order matching
- Partial order execution
- Separate buy and sell order books
- FIFO order matching at each price level
- Object-oriented design using C++

## Tech Stack

- C++
- POSIX Sockets
- STL (`map`, `queue`)
- Object-Oriented Programming

## Project Structure

```
TradeFlow/
│── client.cpp
│── server.cpp
│── Order.h
│── OrderBook.h
│── OrderBook.cpp
```

## How It Works

1. Start the server.
2. Run the client.
3. The client sends a buy or sell order.
4. The server parses the order.
5. The matching engine checks for compatible orders.
6. If a match is found, a trade is executed.
7. The updated order book is displayed.

## Example Orders

Buy Order

```
B 1 100 20
```

Sell Order

```
S 2 100 10
```

## Compilation

Compile the server:

```bash
g++ -std=c++17 server.cpp OrderBook.cpp -o server
```

Compile the client:

```bash
g++ -std=c++17 client.cpp -o client
```

## Run

Start the server:

```bash
./server
```

In another terminal, run the client:

```bash
./client
```

## Sample Execution

The following screenshots demonstrate the execution of the project.

### Server

<img width="403" height="605" alt="Screenshot 2026-07-13 at 8 21 53 PM" src="https://github.com/user-attachments/assets/1b60b7d6-efe0-4ed3-9583-48938ab7eda0" />

### Client 1

<img width="391" height="63" alt="Screenshot 2026-07-13 at 8 22 38 PM" src="https://github.com/user-attachments/assets/6e896b47-b7d4-46f6-8415-326111c4f807" />

### Client 2

<img width="244" height="67" alt="Screenshot 2026-07-13 at 8 22 49 PM" src="https://github.com/user-attachments/assets/fb72782f-e4ca-4c87-9ba4-7231e551bdab" />

## Concepts Demonstrated

- Socket Programming
- Client-Server Architecture
- Order Matching Engine
- Price-Time Priority
- Data Structures
- Object-Oriented Programming
