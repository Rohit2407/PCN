#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8081
#define SIZE 1024

static void timestamp(char *out, size_t n) {
    struct timespec ts;
    struct tm tmv;
    clock_gettime(CLOCK_REALTIME, &ts);
    localtime_r(&ts.tv_sec, &tmv);
    snprintf(out, n, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
        tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday,
        tmv.tm_hour, tmv.tm_min, tmv.tm_sec, ts.tv_nsec / 1000000);
}

int main(void) {
    int server, client;
    struct sockaddr_in addr;
    char buf[SIZE];
    long total_bytes = 0, messages = 0;
    struct timespec start, end;

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) { perror("socket"); return 1; }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server); return 1;
    }
    if (listen(server, 5) < 0) { perror("listen"); close(server); return 1; }

    printf("TCP one-way server listening on port %d\n", PORT);
    client = accept(server, NULL, NULL);
    if (client < 0) { perror("accept"); close(server); return 1; }

    clock_gettime(CLOCK_MONOTONIC, &start);
    while (1) {
        ssize_t n = recv(client, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;
        buf[n] = '\0';
        char ts[64]; timestamp(ts, sizeof(ts));
        printf("[%s] RECEIVED (%zd bytes): %s", ts, n, buf);
        if (buf[n - 1] != '\n') printf("\n");
        total_bytes += n;
        messages++;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\n--- TCP ONE-WAY SERVER METRICS ---\n");
    printf("Received bytes: %ld\nMessages/chunks received: %ld\n", total_bytes, messages);
    printf("Receive duration: %.6f s\n", elapsed);
    if (elapsed > 0) printf("Throughput: %.3f bytes/s (%.3f Kbps)\n",
                            total_bytes / elapsed, (total_bytes * 8.0 / elapsed) / 1000.0);
    close(client); close(server);
    return 0;
}
