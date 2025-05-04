#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

int recv_msg(SOCKET sock, char *buf, struct sockaddr_in *from, int *from_len) {
    int len = recvfrom(sock, buf, 1024 - 1, 0, (struct sockaddr *)from, from_len);
    if (len != SOCKET_ERROR) buf[len] = '\0';
    return len;
}

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server, client, from;
    int from_len = sizeof(from);
    char buf[1024];

    WSAStartup(MAKEWORD(2,2), &wsa);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    client.sin_port = htons(0);
    bind(sock, (struct sockaddr *)&client, sizeof(client));

    server.sin_family = AF_INET;
    server.sin_port = htons(24042);
    server.sin_addr.s_addr = inet_addr("192.168.1.93");

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&(int){16000}, sizeof(int));

    while (1) {
        printf("\nEnter your guess (or 'q' to quit): ");
        fgets(buf, 1024, stdin);
        buf[strcspn(buf, "\n")] = '\0';
        if (strcmp(buf, "q") == 0 || strcmp(buf, "quit") == 0) break;

        sendto(sock, buf, (int)strlen(buf), 0, (struct sockaddr *)&server, sizeof(server));
        printf("Waiting for server response...\n");

        if (recv_msg(sock, buf, &from, &from_len) == SOCKET_ERROR) {
            printf("Timeout or no response. Try again.\n");
            continue;
        }

        printf("%s\n", buf);

        if (strcmp(buf, "You won ?") == 0) {
            sendto(sock, "You won !", 9, 0, (struct sockaddr *)&server, sizeof(server));
            if (recv_msg(sock, buf, &from, &from_len) != SOCKET_ERROR)
                printf("%s\n", strcmp(buf, "You won !") == 0 ? "Congratulations! You won!" : buf);
            else
                printf("No confirmation received.\n");
        } else if (strcmp(buf, "You lost !") == 0) {
            printf("Sorry, you lost the game.\n");
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
