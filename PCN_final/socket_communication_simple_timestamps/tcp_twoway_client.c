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

#define PORT 8082
#define COUNT 10

int main(void)
{
    int sock;
    struct sockaddr_in server;
    char msg[SIZE], reply[SIZE];
    long bytes_sent = 0, bytes_received = 0;
    double total_rtt = 0, max_rtt = 0;
    int completed = 0;
    struct timespec all_start, all_end;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect"); close(sock); return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &all_start);
    for (int i = 1; i <= COUNT; i++) {
        snprintf(msg, sizeof(msg), "TCP message %d", i);
        struct timespec t1, t2;

        show_time("CLIENT SENDING");
        printf("Data: %s\n", msg);
        clock_gettime(CLOCK_MONOTONIC, &t1);

        ssize_t n = send(sock, msg, strlen(msg), 0);
        if (n < 0) { perror("send"); break; }
        bytes_sent += n;

        ssize_t r = recv(sock, reply, sizeof(reply) - 1, 0);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        if (r < 0) { perror("recv"); break; }
        if (r == 0) { printf("Server closed connection\n"); break; }

        reply[r] = '\0';
        bytes_received += r;
        show_time("CLIENT RECEIVED REPLY");
        printf("Reply: %s\n", reply);

        double rtt = elapsed_ms(t1, t2);
        total_rtt += rtt;
        if (rtt > max_rtt) max_rtt = rtt;
        completed++;
        printf("RTT: %.3f ms\n\n", rtt);
    }
    clock_gettime(CLOCK_MONOTONIC, &all_end);

    double elapsed = (all_end.tv_sec - all_start.tv_sec)
                   + (all_end.tv_nsec - all_start.tv_nsec) / 1e9;
    printf("--- TCP TWO-WAY CLIENT METRICS ---\n");
    printf("Bytes sent: %ld\nBytes received: %ld\n", bytes_sent, bytes_received);
    if (completed > 0)
        printf("Average RTT: %.3f ms\nMaximum RTT: %.3f ms\n",
               total_rtt / completed, max_rtt);
    printf("Duration: %.6f seconds\n", elapsed);
    if (elapsed > 0)
        printf("Combined throughput: %.2f bytes/sec\n",
               (bytes_sent + bytes_received) / elapsed);

    close(sock);
    return 0;
}
