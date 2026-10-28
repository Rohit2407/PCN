#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8084
#define SIZE 1024
#define COUNT 10

static void timestamp(char *out, size_t n) {
    struct timespec ts; struct tm tmv;
    clock_gettime(CLOCK_REALTIME,&ts); localtime_r(&ts.tv_sec,&tmv);
    snprintf(out,n,"%04d-%02d-%02d %02d:%02d:%02d.%03ld",
        tmv.tm_year+1900,tmv.tm_mon+1,tmv.tm_mday,tmv.tm_hour,tmv.tm_min,tmv.tm_sec,ts.tv_nsec/1000000);
}
static double diff_ms(struct timespec a, struct timespec b) {
    return (b.tv_sec-a.tv_sec)*1000.0+(b.tv_nsec-a.tv_nsec)/1e6;
}

int main(void) {
    int sock; struct sockaddr_in server; socklen_t slen=sizeof(server); char msg[SIZE],reply[SIZE];
    long sentbytes=0,recvbytes=0; double total=0,max=0; struct timespec allstart,allend;
    sock=socket(AF_INET,SOCK_DGRAM,0); if(sock<0){perror("socket");return 1;}
    memset(&server,0,sizeof(server)); server.sin_family=AF_INET;
    server.sin_port=htons(PORT); inet_pton(AF_INET,"127.0.0.1",&server.sin_addr);
    clock_gettime(CLOCK_MONOTONIC,&allstart);
    for(int i=1;i<=COUNT;i++){
        char ts[64]; snprintf(msg,sizeof(msg),"UDP message %d",i);
        struct timespec t1,t2;
        timestamp(ts,sizeof(ts)); printf("[%s] SENDING packet %d: %s\n",ts,i,msg);
        clock_gettime(CLOCK_MONOTONIC,&t1);
        ssize_t n=sendto(sock,msg,strlen(msg),0,(struct sockaddr *)&server,sizeof(server));
        if(n<0){perror("sendto");break;} sentbytes+=n;
        ssize_t r=recvfrom(sock,reply,sizeof(reply)-1,0,NULL,NULL);
        clock_gettime(CLOCK_MONOTONIC,&t2);
        if(r<0){perror("recvfrom");break;} reply[r]='\0'; recvbytes+=r;
        timestamp(ts,sizeof(ts)); printf("[%s] RECEIVED echo: %s\n",ts,reply);
        double rtt=diff_ms(t1,t2); total+=rtt; if(rtt>max)max=rtt;
        printf("RTT for packet %d: %.3f ms\n\n",i,rtt);
    }
    clock_gettime(CLOCK_MONOTONIC,&allend);
    double elapsed=(allend.tv_sec-allstart.tv_sec)+(allend.tv_nsec-allstart.tv_nsec)/1e9;
    printf("--- UDP TWO-WAY CLIENT METRICS ---\nBytes sent: %ld\nBytes received: %ld\n",sentbytes,recvbytes);
    printf("Average RTT: %.3f ms\nMaximum RTT: %.3f ms\n",total/COUNT,max);
    printf("Total duration: %.6f s\n",elapsed);
    if(elapsed>0)printf("Combined throughput: %.3f bytes/s\n",(sentbytes+recvbytes)/elapsed);
    close(sock); return 0;
}
