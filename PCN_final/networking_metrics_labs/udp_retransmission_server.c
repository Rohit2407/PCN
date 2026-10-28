#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9001
#define SIZE 1024

int main(void)
{
    int sock;
    struct sockaddr_in server, client;
    socklen_t len;
    char buf[SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind"); close(sock); return 1;
    }

    printf("UDP retransmission demo server on port %d\n", PORT);
    printf("Waits for 5 unique sequence numbers; duplicate packets are acknowledged too.\n");

    int seen[6] = {0};
    int unique = 0;
    while (unique < 5) {
        len = sizeof(client);
        ssize_t n = recvfrom(sock, buf, sizeof(buf) - 1, 0,
                             (struct sockaddr *)&client, &len);
        if (n < 0) { perror("recvfrom"); continue; }
        buf[n] = '\0';

        int seq = 0;
        if (sscanf(buf, "SEQ %d", &seq) != 1 || seq < 1 || seq > 5) {
            printf("Invalid packet: %s\n", buf);
            continue;
        }

        if (!seen[seq]) {
            seen[seq] = 1;
            unique++;
            printf("Received new packet seq=%d (%zd bytes)\n", seq, n);
        } else {
            printf("Received RETRANSMISSION duplicate seq=%d (%zd bytes)\n", seq, n);
        }

        char ack[64];
        snprintf(ack, sizeof(ack), "ACK %d", seq);
        if (sendto(sock, ack, strlen(ack), 0,
                   (struct sockaddr *)&client, len) < 0)
            perror("sendto");
    }

    printf("All 5 unique packets received. Server exiting.\n");
    close(sock);
    return 0;
}
