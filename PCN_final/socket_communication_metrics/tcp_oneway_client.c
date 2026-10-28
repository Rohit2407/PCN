#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8081
#define SIZE 1024
#define COUNT 10

static void timestamp(char *out, size_t n) {
    struct timespec ts; struct tm tmv;
    clock_gettime(CLOCK_REALTIME, &ts); localtime_r(&ts.tv_sec, &tmv);
    snprintf(out, n, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
        tmv.tm_year+1900, tmv.tm_mon+1, tmv.tm_mday,
        tmv.tm_hour, tmv.tm_min, tmv.tm_sec, ts.tv_nsec/1000000);
}

int main(void) {
    int sock; struct sockaddr_in server; char msg[SIZE];
    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET; server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect"); close(sock); return 1;
    }

    for (int i = 1; i <= COUNT; i++) {
        char ts[64]; timestamp(ts, sizeof(ts));
        snprintf(msg, sizeof(msg), "Message %d sent at %s\n", i, ts);
        ssize_t n = send(sock, msg, strlen(msg), 0);
        if (n < 0) { perror("send"); break; }
        timestamp(ts, sizeof(ts));
        printf("[%s] SENT (%zd bytes): %s", ts, n, msg);
        usleep(200000);
    }
    shutdown(sock, SHUT_WR);
    close(sock);
    return 0;
}
