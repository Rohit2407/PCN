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

#define PORT 8083
#define COUNT 10

int main(void)
{
    int sock;
    struct sockaddr_in server;
    char msg[SIZE];
    long total_bytes = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    for (int i = 1; i <= COUNT; i++) {
        snprintf(msg, sizeof(msg), "UDP message %d", i);
        show_time("SENDING");
        printf("Packet %d data: %s\n", i, msg);

        ssize_t n = sendto(sock, msg, strlen(msg), 0,
                           (struct sockaddr *)&server, sizeof(server));
        if (n < 0) { perror("sendto"); break; }
        total_bytes += n;
        usleep(200000);
    }

    printf("Total payload bytes sent: %ld\n", total_bytes);
    close(sock);
    return 0;
}
