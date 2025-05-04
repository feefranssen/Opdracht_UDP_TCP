#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    int target, guess;
    char response[32];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(65432);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        printf("Listen failed.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server luistert op poort %d...\n", 65432);
    srand((unsigned int)time(NULL));

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            printf("Accept failed.\n");
            continue;
        }

        printf("Client verbonden.\n");
        target = rand() % 100 + 1;
        printf("Te raden getal: %d\n", target);

        while (1) {
            int net_guess;
            int bytesReceived = recv(clientSocket, (char*)&net_guess, sizeof(net_guess), 0);
            if (bytesReceived <= 0) break;

            guess = ntohl(net_guess);
            printf("Client gokt: %d\n", guess);

            if (guess < target) {
                strcpy(response, "Hoger");
            } else if (guess > target) {
                strcpy(response, "Lager");
            } else {
                strcpy(response, "Correct");
                send(clientSocket, response, strlen(response), 0);
                break;
            }

            send(clientSocket, response, strlen(response), 0);
        }

        printf("Client sessie afgesloten.\n");
        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
