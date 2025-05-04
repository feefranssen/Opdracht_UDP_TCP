#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    int guess;
    char buffer[32];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }


    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(65432);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.1.93");


    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Verbinding met server mislukt.\n");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    printf("Verbonden met de server. Raad het getal tussen 1 en 100!\n");

    while (1) {
        printf("Jouw gok: ");
        scanf("%d", &guess);

        int net_guess = htonl(guess); 
        send(clientSocket, (char*)&net_guess, sizeof(net_guess), 0);

        int bytesReceived = recv(clientSocket, buffer, 32 - 1, 0);
        if (bytesReceived <= 0) break;

        buffer[bytesReceived] = '\0';
        printf("Server zegt: %s\n", buffer);

        if (strcmp(buffer, "Correct") == 0) {
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
