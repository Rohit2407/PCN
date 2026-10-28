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

#define PORT 8081

int main(void)
{
    int server, client;
    struct sockaddr_in addr;
    char buf[SIZE];
    long total_bytes = 0, chunks = 0;
    struct timespec start, end;

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) { perror("socket"); return 1; }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server); return 1;
    }
    if (listen(server, 5) < 0) {
        perror("listen"); close(server); return 1;
    }

    printf("TCP one-way server on port %d\n", PORT);
    client = accept(server, NULL, NULL);
    if (client < 0) { perror("accept"); close(server); return 1; }

    clock_gettime(CLOCK_MONOTONIC, &start);
    while (1) {
        ssize_t n = recv(client, buf, sizeof(buf) - 1, 0);
        if (n < 0) { perror("recv"); break; }
        if (n == 0) break;
        buf[n] = '\0';
        show_time("RECEIVED");
        printf("Data: %s", buf);
        if (buf[n - 1] != '\n') printf("\n");
        total_bytes += n;
        chunks++;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec)
                   + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\n--- TCP ONE-WAY METRICS ---\n");
    printf("Received bytes: %ld\nReceived chunks: %ld\n", total_bytes, chunks);
    printf("Duration: %.6f seconds\n", elapsed);
    if (elapsed > 0)
        printf("Throughput: %.2f bytes/sec, %.2f Kbps\n",
               total_bytes / elapsed, total_bytes * 8.0 / elapsed / 1000.0);

    close(client);
    close(server);
    return 0;
}
