#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define SIZE 1024

int main(void)
{
    int sock;
    struct sockaddr_in server;
    char request[SIZE];
    char response[SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    strcpy(request,
           "GET /index.html HTTP/1.1\r\n"
           "Host: localhost\r\n"
           "Connection: close\r\n\r\n");

    if (send(sock, request, strlen(request), 0) < 0) {
        perror("send");
        close(sock);
        return 1;
    }

    FILE *fp = fopen("downloaded_page.html", "wb");
    if (fp == NULL) {
        perror("fopen");
        close(sock);
        return 1;
    }

    int n;
    while ((n = recv(sock, response, sizeof(response), 0)) > 0) {
        fwrite(response, 1, n, fp);
        printf("%.*s", n, response);
    }

    if (n < 0)
        perror("recv");

    fclose(fp);
    close(sock);

    printf("\n\nHTTP response saved to downloaded_page.html\n");
    printf("Note: this file contains HTTP headers plus the HTML body.\n");
    return 0;
}
