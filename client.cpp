#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

int main()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        cout << "Socket creation failed!\n";
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
    {
        cout << "Connection failed!\n";
        return 1;
    }

    string message;
    cout << "Enter Order (format: B/S id price qty), e.g. \"B 1 100 50\": ";
    getline(cin, message);

    send(clientSocket, message.c_str(), message.length(), 0);

    char buffer[1024] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0)
    {
        buffer[bytesReceived] = '\0';
        cout << "Server response: " << buffer << endl;
    }

    close(clientSocket);
    return 0;
}
