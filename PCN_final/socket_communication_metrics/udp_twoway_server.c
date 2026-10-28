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

int main(void) {
    int sock; struct sockaddr_in server,client; socklen_t len; char buf[SIZE];
    long in=0,out=0; struct timespec start,end;
    sock=socket(AF_INET,SOCK_DGRAM,0); if(sock<0){perror("socket");return 1;}
    memset(&server,0,sizeof(server)); server.sin_family=AF_INET;
    server.sin_port=htons(PORT); server.sin_addr.s_addr=INADDR_ANY;
    if(bind(sock,(struct sockaddr *)&server,sizeof(server))<0){perror("bind");return 1;}
    printf("UDP two-way echo server on port %d; waiting for %d packets\n",PORT,COUNT);
    clock_gettime(CLOCK_MONOTONIC,&start);
    for(int i=1;i<=COUNT;i++){
        len=sizeof(client);
        ssize_t n=recvfrom(sock,buf,sizeof(buf),0,(struct sockaddr *)&client,&len);
        if(n<0){perror("recvfrom");continue;}
        char ts[64]; timestamp(ts,sizeof(ts)); printf("[%s] RECEIVED packet %d (%zd bytes)\n",ts,i,n); in+=n;
        ssize_t s=sendto(sock,buf,n,0,(struct sockaddr *)&client,len);
        if(s<0){perror("sendto");continue;}
        timestamp(ts,sizeof(ts)); printf("[%s] SENT echo %d (%zd bytes)\n",ts,i,s); out+=s;
    }
    clock_gettime(CLOCK_MONOTONIC,&end);
    double elapsed=(end.tv_sec-start.tv_sec)+(end.tv_nsec-start.tv_nsec)/1e9;
    printf("\n--- UDP TWO-WAY SERVER METRICS ---\nBytes received: %ld\nBytes sent: %ld\nDuration: %.6f s\n",in,out,elapsed);
    if(elapsed>0) printf("Combined throughput: %.3f bytes/s\n",(in+out)/elapsed);
    close(sock); return 0;
}
