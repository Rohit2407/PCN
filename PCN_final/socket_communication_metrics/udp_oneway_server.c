#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8083
#define SIZE 1024

static void timestamp(char *out, size_t n) {
    struct timespec ts; struct tm tmv;
    clock_gettime(CLOCK_REALTIME, &ts); localtime_r(&ts.tv_sec, &tmv);
    snprintf(out, n, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
        tmv.tm_year+1900, tmv.tm_mon+1, tmv.tm_mday,
        tmv.tm_hour, tmv.tm_min, tmv.tm_sec, ts.tv_nsec/1000000);
}

int main(void) {
    int sock; struct sockaddr_in addr, client; socklen_t len;
    char buf[SIZE]; long bytes=0, packets=0; struct timespec start, end;
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    memset(&addr,0,sizeof(addr)); addr.sin_family=AF_INET;
    addr.sin_port=htons(PORT); addr.sin_addr.s_addr=INADDR_ANY;
    if (bind(sock,(struct sockaddr *)&addr,sizeof(addr))<0) { perror("bind"); return 1; }
    printf("UDP one-way server listening on port %d; waiting for 10 packets\n", PORT);
    clock_gettime(CLOCK_MONOTONIC,&start);
    for (int i=0;i<10;i++) {
        len=sizeof(client);
        ssize_t n=recvfrom(sock,buf,sizeof(buf)-1,0,(struct sockaddr *)&client,&len);
        if (n<0) { perror("recvfrom"); continue; }
        buf[n]='\0'; char ts[64]; timestamp(ts,sizeof(ts));
        printf("[%s] RECEIVED packet %d (%zd bytes): %s\n",ts,i+1,n,buf);
        bytes+=n; packets++;
    }
    clock_gettime(CLOCK_MONOTONIC,&end);
    double elapsed=(end.tv_sec-start.tv_sec)+(end.tv_nsec-start.tv_nsec)/1e9;
    printf("\n--- UDP ONE-WAY SERVER METRICS ---\nPackets received: %ld\nPayload bytes received: %ld\nDuration: %.6f s\n",packets,bytes,elapsed);
    if (elapsed>0) printf("Throughput: %.3f bytes/s (%.3f Kbps)\n",bytes/elapsed,(bytes*8.0/elapsed)/1000.0);
    close(sock); return 0;
}
