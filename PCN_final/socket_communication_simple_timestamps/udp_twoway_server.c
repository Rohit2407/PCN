#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SIZE 1024

void show_time(const char *label)
{
    time_t t = time(NULL);
    printf("%s (time): %s", label, ctime(&t));

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    printf("%s (clock_gettime): %ld.%09ld seconds\n",
           label, (long)ts.tv_sec, ts.tv_nsec);
}

double elapsed_ms(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) * 1000.0
         + (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

#define PORT 8084
#define COUNT 10

int main(void)
{
    int sock;
    struct sockaddr_in server, client;
    socklen_t len;
    char buf[SIZE];
    long bytes_in = 0, bytes_out = 0;
    struct timespec start, end;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind"); close(sock); return 1;
    }

    printf("UDP two-way echo server on port %d; waiting for %d packets\n", PORT, COUNT);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 1; i <= COUNT; i++) {
        len = sizeof(client);
        ssize_t n = recvfrom(sock, buf, sizeof(buf), 0,
                             (struct sockaddr *)&client, &len);
        if (n < 0) { perror("recvfrom"); continue; }

        show_time("SERVER RECEIVED");
        printf("Packet %d, %zd bytes\n", i, n);
        bytes_in += n;

        ssize_t sent = sendto(sock, buf, n, 0,
                              (struct sockaddr *)&client, len);
        if (sent < 0) { perror("sendto"); continue; }
        show_time("SERVER SENT REPLY");
        printf("Reply %d, %zd bytes\n", i, sent);
        bytes_out += sent;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec)
                   + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\n--- UDP TWO-WAY SERVER METRICS ---\n");
    printf("Bytes received: %ld\nBytes sent: %ld\n", bytes_in, bytes_out);
    printf("Duration: %.6f seconds\n", elapsed);
    if (elapsed > 0)
        printf("Combined throughput: %.2f bytes/sec\n",
               (bytes_in + bytes_out) / elapsed);

    close(sock);
    return 0;
}
