#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define SIZE 2048

int main(void)
{
    int sock;
    struct sockaddr_in server;
    char request[] = "GET_HTML";
    char response[SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (sendto(sock, request, strlen(request), 0,
               (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("sendto");
        close(sock);
        return 1;
    }

    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        perror("setsockopt");

    ssize_t n = recvfrom(sock, response, sizeof(response), 0, NULL, NULL);
    if (n < 0) {
        perror("recvfrom");
        close(sock);
        return 1;
    }

    FILE *fp = fopen("udp_downloaded_page.html", "wb");
    if (fp == NULL) {
        perror("fopen");
        close(sock);
        return 1;
    }

    fwrite(response, 1, n, fp);
    fclose(fp);

    printf("Received %zd bytes of HTML over custom UDP.\n", n);
    printf("Saved to udp_downloaded_page.html\n");
    printf("This is a simple demo: one UDP datagram only; large pages may be truncated.\n");

    close(sock);
    return 0;
}
