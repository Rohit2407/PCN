#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PORT 5000
int main(void) {
    int fd = socket(AF_INET, SOCK_STREAM, 0); struct sockaddr_in addr;
    char msg[] = "Hello over TCP", reply[4096] = {0};
    struct timespec a,b;
    memset(&addr,0,sizeof(addr)); addr.sin_family=AF_INET;
    addr.sin_port=htons(PORT); inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr);
    clock_gettime(CLOCK_MONOTONIC,&a);
    if (connect(fd,(struct sockaddr*)&addr,sizeof(addr))<0) { perror("connect"); return 1; }
    send(fd,msg,strlen(msg),0);
    ssize_t n=recv(fd,reply,sizeof(reply)-1,0);
    clock_gettime(CLOCK_MONOTONIC,&b);
    double sec=(b.tv_sec-a.tv_sec)+(b.tv_nsec-a.tv_nsec)/1e9;
    if(n>0) printf("Reply: %s\n",reply);
    printf("Request/response RTT approximation: %.3f ms\n",sec*1000);
    printf("Reply throughput: %.6f Mbps\n",sec>0?(n*8.0/sec/1e6):0);
    close(fd); return 0;
}
