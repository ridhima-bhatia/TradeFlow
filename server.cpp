#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#include "OrderBook.h"

using namespace std;

int main()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1)
    {
        cout << "Socket creation failed!\n";
        return 1;
    }

    cout << "Socket created successfully!\n";

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (::bind(serverSocket,
               (sockaddr*)&serverAddress,
               sizeof(serverAddress)) == -1)
    {
        cout << "Bind failed!\n";
        return 1;
    }

    cout << "Socket bound successfully!\n";

    if (listen(serverSocket, SOMAXCONN) == -1)
    {
        cout << "Listen failed!\n";
        return 1;
    }

    cout << "Server is listening on port 8080...\n";

    OrderBook book;

    while (true)
    {
        cout << "\nWaiting for a client...\n";

        int clientSocket = accept(serverSocket, nullptr, nullptr);

        if (clientSocket == -1)
        {
            cout << "Accept failed!\n";
            continue;
        }

        cout << "Client connected!\n";

        char buffer[1024] = {0};

        int bytesReceived = recv(clientSocket,
                                 buffer,
                                 sizeof(buffer),
                                 0);

        if (bytesReceived <= 0)
        {
            cout << "No message received.\n";
            close(clientSocket);
            continue;
        }

        cout << "\nReceived : " << buffer << endl;

        stringstream ss(buffer);

        char side;
        int id;
        int price;
        int quantity;

        ss >> side >> id >> price >> quantity;

        Side orderSide;

        if (side == 'B')
            orderSide = Side::BUY;
        else
            orderSide = Side::SELL;

        Order order(id,
                    orderSide,
                    price,
                    quantity);

        book.addOrder(order);

        book.printOrderBook();

        close(clientSocket);

        cout << "\nClient disconnected.\n";
    }

    close(serverSocket);

    return 0;
}