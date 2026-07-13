#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

int main()
{
    // ============================================
    // Step 1: Create Client Socket
    // ============================================
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == -1)
    {
        cout << "Socket creation failed!\n";
        return 1;
    }

    cout << "Client socket created.\n";


    // ============================================
    // Step 2: Configure Server Address
    // ============================================
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");


    // ============================================
    // Step 3: Connect to Server
    // ============================================
    if (connect(clientSocket,
                (sockaddr*)&serverAddress,
                sizeof(serverAddress)) == -1)
    {
        cout << "Connection failed!\n";
        return 1;
    }

    cout << "Connected to server!\n";


    // ============================================
    // Step 4: Send Order
    // ============================================
    string message;

    cout << "Enter Order: ";
    getline(cin, message);

    send(clientSocket,
         message.c_str(),
         message.length(),
         0);

    cout << "Order sent successfully!\n";


    // ============================================
    // Step 5: Close Socket
    // ============================================
    close(clientSocket);

    return 0;
}