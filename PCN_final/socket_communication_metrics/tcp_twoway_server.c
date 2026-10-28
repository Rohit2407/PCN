#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define SIZE 1024

static void timestamp(char *out, size_t n) {
    struct timespec ts; struct tm tmv;
    clock_gettime(CLOCK_REALTIME, &ts); localtime_r(&ts.tv_sec, &tmv);
    snprintf(out, n, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
        tmv.tm_year+1900, tmv.tm_mon+1, tmv.tm_mday,
        tmv.tm_hour, tmv.tm_min, tmv.tm_sec, ts.tv_nsec/1000000);
}

int main(void) {
    int server, client; struct sockaddr_in addr; char buf[SIZE];
    long bytes_in = 0, bytes_out = 0, messages = 0;
    struct timespec start, end;
    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) { perror("socket"); return 1; }
    memset(&addr, 0, sizeof(addr)); addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT); addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (listen(server, 5) < 0) { perror("listen"); return 1; }
    printf("TCP two-way (echo) server listening on port %d\n", PORT);
    client = accept(server, NULL, NULL);
    if (client < 0) { perror("accept"); return 1; }
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (1) {
        ssize_t n = recv(client, buf, sizeof(buf), 0);
        if (n <= 0) break;
        char ts[64]; timestamp(ts, sizeof(ts));
        printf("[%s] RECEIVED %zd bytes\n", ts, n);
        bytes_in += n; messages++;
        ssize_t sent = send(client, buf, n, 0);
        if (sent < 0) { perror("send"); break; }
        timestamp(ts, sizeof(ts));
        printf("[%s] SENT REPLY %zd bytes\n", ts, sent);
        bytes_out += sent;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec-start.tv_sec)+(end.tv_nsec-start.tv_nsec)/1e9;
    printf("\n--- TCP TWO-WAY SERVER METRICS ---\n");
    printf("Messages/chunks handled: %ld\nBytes received: %ld\nBytes sent: %ld\n", messages, bytes_in, bytes_out);
    printf("Duration: %.6f s\n", elapsed);
    if (elapsed > 0) printf("Receive throughput: %.3f bytes/s (%.3f Kbps)\n",
        bytes_in/elapsed, (bytes_in*8.0/elapsed)/1000.0);
    close(client); close(server); return 0;
}
