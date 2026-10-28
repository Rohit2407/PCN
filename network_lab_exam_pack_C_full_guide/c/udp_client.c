#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PORT 5001
int main(void) {
    int fd=socket(AF_INET,SOCK_DGRAM,0); struct sockaddr_in addr;
    char msg[]="Hello over UDP",reply[4096]={0}; struct timeval tv={2,0};
    setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
    memset(&addr,0,sizeof(addr));addr.sin_family=AF_INET;addr.sin_port=htons(PORT);
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr);
    struct timespec a,b;clock_gettime(CLOCK_MONOTONIC,&a);
    sendto(fd,msg,strlen(msg),0,(struct sockaddr*)&addr,sizeof(addr));
    ssize_t n=recvfrom(fd,reply,sizeof(reply)-1,0,NULL,NULL);
    clock_gettime(CLOCK_MONOTONIC,&b);
    double sec=(b.tv_sec-a.tv_sec)+(b.tv_nsec-a.tv_nsec)/1e9;
    if(n>0){reply[n]='\0';printf("Reply: %s\n",reply);printf("RTT: %.3f ms\n",sec*1000);printf("PDR for one request: 100%%\n");}
    else printf("No reply (timeout); PDR for one request: 0%%\n");
    strcpy(msg,"quit");sendto(fd,msg,strlen(msg),0,(struct sockaddr*)&addr,sizeof(addr));
    close(fd);return 0;
}
