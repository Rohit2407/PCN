#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define SIZE 2048

int main(void)
{
    int sock;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);
    char request[SIZE];
    char page[SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    printf("Custom UDP HTML server running on port %d\n", PORT);

    FILE *fp = fopen("index.html", "rb");
    if (fp == NULL) {
        perror("fopen index.html");
        close(sock);
        return 1;
    }

    size_t count = fread(page, 1, sizeof(page), fp);
    fclose(fp);

    while (1) {
        memset(request, 0, sizeof(request));
        ssize_t n = recvfrom(sock, request, sizeof(request) - 1, 0,
                             (struct sockaddr *)&client, &len);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        request[n] = '\0';
        printf("Received UDP request: %s\n", request);

        ssize_t sent = sendto(sock, page, count, 0,
                              (struct sockaddr *)&client, len);
        if (sent < 0)
            perror("sendto");
        else
            printf("Sent %zd bytes of HTML over UDP\n", sent);
    }

    close(sock);
    return 0;
}
