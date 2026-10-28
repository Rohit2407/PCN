#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define SIZE 1024
#define COUNT 10

static void timestamp(char *out, size_t n) {
    struct timespec ts; struct tm tmv;
    clock_gettime(CLOCK_REALTIME, &ts); localtime_r(&ts.tv_sec, &tmv);
    snprintf(out, n, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
        tmv.tm_year+1900, tmv.tm_mon+1, tmv.tm_mday,
        tmv.tm_hour, tmv.tm_min, tmv.tm_sec, ts.tv_nsec/1000000);
}
static double diff_ms(struct timespec a, struct timespec b) {
    return (b.tv_sec-a.tv_sec)*1000.0 + (b.tv_nsec-a.tv_nsec)/1e6;
}

int main(void) {
    int sock; struct sockaddr_in server; char msg[SIZE], reply[SIZE];
    double total_rtt = 0, max_rtt = 0; long bytes_sent = 0, bytes_received = 0;
    struct timespec all_start, all_end;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    memset(&server, 0, sizeof(server)); server.sin_family = AF_INET;
    server.sin_port = htons(PORT); inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) { perror("connect"); return 1; }
    clock_gettime(CLOCK_MONOTONIC, &all_start);
    for (int i=1; i<=COUNT; i++) {
        char ts[64]; snprintf(msg, sizeof(msg), "TCP message %d", i);
        struct timespec t1, t2;
        timestamp(ts, sizeof(ts)); printf("[%s] SENDING: %s\n", ts, msg);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        ssize_t n = send(sock, msg, strlen(msg), 0);
        if (n < 0) { perror("send"); break; }
        bytes_sent += n;
        ssize_t r = recv(sock, reply, sizeof(reply)-1, 0);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        if (r <= 0) { perror("recv"); break; }
        reply[r]='\0'; bytes_received += r;
        timestamp(ts, sizeof(ts)); printf("[%s] RECEIVED REPLY: %s\n", ts, reply);
        double rtt = diff_ms(t1,t2); total_rtt += rtt; if (rtt > max_rtt) max_rtt = rtt;
        printf("RTT for message %d: %.3f ms\n\n", i, rtt);
    }
    clock_gettime(CLOCK_MONOTONIC, &all_end);
    double elapsed=(all_end.tv_sec-all_start.tv_sec)+(all_end.tv_nsec-all_start.tv_nsec)/1e9;
    printf("--- TCP TWO-WAY CLIENT METRICS ---\n");
    printf("Bytes sent: %ld\nBytes received: %ld\n", bytes_sent, bytes_received);
    printf("Average RTT: %.3f ms\nMaximum RTT: %.3f ms\n", COUNT ? total_rtt/COUNT : 0, max_rtt);
    printf("Total duration: %.6f s\n", elapsed);
    if (elapsed > 0) printf("Combined throughput: %.3f bytes/s\n", (bytes_sent+bytes_received)/elapsed);
    close(sock); return 0;
}
