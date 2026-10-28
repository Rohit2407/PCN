#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 9001
#define SIZE 1024
#define COUNT 5
#define MAX_RETRIES 4
#define TIMEOUT_SEC 1

void show_time(const char *label)
{
    time_t t = time(NULL);
    printf("%s: %s", label, ctime(&t));
}

double diff_ms(struct timespec a, struct timespec b)
{
    return (b.tv_sec - a.tv_sec) * 1000.0
         + (b.tv_nsec - a.tv_nsec) / 1000000.0;
}

int main(void)
{
    int sock;
    struct sockaddr_in server;
    char msg[SIZE], ack[SIZE];
    long bytes_sent = 0, bytes_received = 0, retransmissions = 0;
    double rtt_total = 0, rtt_max = 0;
    int acknowledged = 0;
    struct timespec all_start, all_end;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt"); close(sock); return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &all_start);

    for (int seq = 1; seq <= COUNT; seq++) {
        int got_ack = 0;
        for (int attempt = 0; attempt <= MAX_RETRIES && !got_ack; attempt++) {
            snprintf(msg, sizeof(msg), "SEQ %d Hello packet %d", seq, seq);
            struct timespec t1, t2;
            if (attempt > 0) {
                retransmissions++;
                printf("Timeout: retransmitting seq=%d (attempt %d)\n", seq, attempt);
            }

            show_time("SEND timestamp");
            clock_gettime(CLOCK_MONOTONIC, &t1);
            ssize_t n = sendto(sock, msg, strlen(msg), 0,
                               (struct sockaddr *)&server, sizeof(server));
            if (n < 0) { perror("sendto"); close(sock); return 1; }
            bytes_sent += n;

            ssize_t r = recvfrom(sock, ack, sizeof(ack) - 1, 0, NULL, NULL);
            clock_gettime(CLOCK_MONOTONIC, &t2);

            if (r < 0) {
                if (attempt == MAX_RETRIES) {
                    printf("No ACK for seq=%d after %d retries\n", seq, MAX_RETRIES);
                }
                continue;
            }

            ack[r] = '\0';
            bytes_received += r;
            char expected[64];
            snprintf(expected, sizeof(expected), "ACK %d", seq);
            if (strcmp(ack, expected) == 0) {
                double rtt = diff_ms(t1, t2);
                rtt_total += rtt;
                if (rtt > rtt_max) rtt_max = rtt;
                acknowledged++;
                got_ack = 1;
                show_time("ACK receive timestamp");
                printf("Received %s; RTT = %.3f ms\n\n", ack, rtt);
            } else {
                printf("Unexpected ACK: %s\n", ack);
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &all_end);
    double elapsed = (all_end.tv_sec - all_start.tv_sec)
                   + (all_end.tv_nsec - all_start.tv_nsec) / 1e9;

    printf("\n--- RETRANSMISSION METRICS ---\n");
    printf("Original packets: %d\nACKed packets: %d\n", COUNT, acknowledged);
    printf("Retransmissions: %ld\n", retransmissions);
    printf("Payload bytes sent (includes retransmissions): %ld\n", bytes_sent);
    printf("ACK bytes received: %ld\n", bytes_received);
    printf("Packet delivery ratio: %.2f%%\n", 100.0 * acknowledged / COUNT);
    if (acknowledged > 0)
        printf("Average RTT: %.3f ms\nMaximum RTT: %.3f ms\n",
               rtt_total / acknowledged, rtt_max);
    printf("Total elapsed time: %.3f s\n", elapsed);
    if (elapsed > 0)
        printf("Sender throughput incl. retransmissions: %.2f bytes/s\n",
               bytes_sent / elapsed);

    close(sock);
    return 0;
}
