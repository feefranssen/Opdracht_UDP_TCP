#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

int compare_sockaddr(struct sockaddr_in *a, struct sockaddr_in *b) {
    return a->sin_addr.s_addr == b->sin_addr.s_addr && a->sin_port == b->sin_port;
}

int main() {
    WSADATA wsaData;
    SOCKET server_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);
    char buffer[1024];
    int random_number;
    int best_guess = -1;
    struct sockaddr_in best_client;
    int best_diff = 999999;
    int timeout;
    int recv_len;

    struct sockaddr_in all_clients[64];
    int client_count;

    srand((unsigned int)time(NULL));
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket == INVALID_SOCKET) {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(24042);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Server started on port %d...\n", 24042);

    while (1) {
        random_number = rand() % 100;
        printf("\nRandom number: %d\n", random_number);

        best_guess = -1;
        best_diff = 999999;
        client_count = 0;
        timeout = 8000;

        setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

        DWORD start_time = GetTickCount();
        DWORD elapsed_time = 0;

        while (elapsed_time < 16000) {
            recv_len = recvfrom(server_socket, buffer, sizeof(buffer) - 1, 0,
                                (struct sockaddr *)&client_addr, &client_addr_len);

            if (recv_len == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err == WSAETIMEDOUT) {
                    printf("Receive timeout.\n");
                } else {
                    printf("Recvfrom failed with error: %d\n", err);
                }
                break;
            }
            buffer[recv_len] = '\0';
            int guess = atoi(buffer);
            printf("Received guess %d from %s:%d\n", guess,
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            int diff = abs(guess - random_number);

            int known = 0;
            for (int i = 0; i < client_count; i++) {
                if (compare_sockaddr(&all_clients[i], &client_addr)) {
                    known = 1;
                    break;
                }
            }
            if (!known && client_count < 64) {
                memcpy(&all_clients[client_count++], &client_addr, sizeof(client_addr));
            }
            if (diff < best_diff) {
                best_diff = diff;
                best_guess = guess;
                memcpy(&best_client, &client_addr, sizeof(best_client));

                timeout /= 2;
                if (timeout < 500) timeout = 500;
                setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
                printf("New best guess: %d (diff: %d), timeout set to %d ms\n", best_guess, best_diff, timeout);
            }

            elapsed_time = GetTickCount() - start_time;
        }

        if (best_guess != -1) {
            sendto(server_socket, "You won ?", (int)strlen("You won ?"), 0,
                   (struct sockaddr *)&best_client, sizeof(best_client));
            printf("Sent 'You won ?' to %s:%d\n",
                   inet_ntoa(best_client.sin_addr), ntohs(best_client.sin_port));

            sendto(server_socket, "You won !", (int)strlen("You won !"), 0,
                   (struct sockaddr *)&best_client, sizeof(best_client));
            printf("Confirmed winner with 'You won !' to %s:%d\n",
                   inet_ntoa(best_client.sin_addr), ntohs(best_client.sin_port));

            for (int i = 0; i < client_count; i++) {
                if (!compare_sockaddr(&all_clients[i], &best_client)) {
                    sendto(server_socket, "You lost !", (int)strlen("You lost !"), 0,
                           (struct sockaddr *)&all_clients[i], sizeof(all_clients[i]));
                    printf("Sent 'You lost !' to %s:%d\n",
                           inet_ntoa(all_clients[i].sin_addr), ntohs(all_clients[i].sin_port));
                }
            }
        } else {
            printf("No guesses received in time.\n");
        }
    }

    closesocket(server_socket);
    WSACleanup();

    return 0;
}
