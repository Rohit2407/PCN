// TCP bulk transfer: run ./tcp_transfer_metrics server, then ./tcp_transfer_metrics client [bytes]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PORT 5011
int main(int argc,char **argv){
    if(argc<2){printf("Usage: %s server|client [bytes]\n",argv[0]);return 1;}
    int fd=socket(AF_INET,SOCK_STREAM,0);struct sockaddr_in addr;memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);addr.sin_port=htons(PORT);
    if(strcmp(argv[1],"server")==0){
        int opt=1;setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
        if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))<0||listen(fd,1)<0){perror("server");return 1;}
        printf("TCP transfer receiver on port %d\n",PORT);int c=accept(fd,NULL,NULL);char buf[65536];long long total=0;
        ssize_t n;while((n=recv(c,buf,sizeof(buf),0))>0)total+=n;
        char out[64];snprintf(out,sizeof(out),"%lld",total);send(c,out,strlen(out),0);
        printf("Bytes received: %lld\n",total);close(c);
    }else{
        long long bytes=argc>2?atoll(argv[2]):1000000;char *buf=malloc(65536);memset(buf,'x',65536);
        struct timespec a,b;clock_gettime(CLOCK_MONOTONIC,&a);
        if(connect(fd,(struct sockaddr*)&addr,sizeof(addr))<0){perror("connect");return 1;}
        long long sent=0;while(sent<bytes){size_t n=(bytes-sent>65536)?65536:(size_t)(bytes-sent);
            if(send(fd,buf,n,0)<0){perror("send");return 1;}sent+=n;}
        shutdown(fd,SHUT_WR);char reply[64]={0};recv(fd,reply,sizeof(reply)-1,0);
        clock_gettime(CLOCK_MONOTONIC,&b);double sec=(b.tv_sec-a.tv_sec)+(b.tv_nsec-a.tv_nsec)/1e9;
        printf("Receiver-confirmed bytes: %s\nElapsed: %.4f s\nGoodput: %.4f Mbps\n",reply,sec,sec>0?sent*8.0/sec/1e6:0);
        printf("TCP retransmissions are handled by the OS TCP stack; inspect packet capture/OS TCP statistics to count them.\n");
        free(buf);
    }close(fd);return 0;
}
