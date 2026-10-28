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
#define COUNT 10

int main(void)
{
    int sock;
    struct sockaddr_in server;
    char msg[SIZE];
    long total_bytes = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect"); close(sock); return 1;
    }

    for (int i = 1; i <= COUNT; i++) {
        snprintf(msg, sizeof(msg), "TCP message %d\n", i);
        show_time("SENDING");
        ssize_t n = send(sock, msg, strlen(msg), 0);
        if (n < 0) { perror("send"); break; }
        total_bytes += n;
        printf("Data sent: %s", msg);
        usleep(200000);
    }

    printf("Total bytes sent: %ld\n", total_bytes);
    shutdown(sock, SHUT_WR);
    close(sock);
    return 0;
}
