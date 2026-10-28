#include <stdio.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
int main(void){
    char host[256];struct addrinfo hints,*res,*p;struct timespec a,b;
    printf("Hostname to resolve (e.g. example.com): ");if(scanf("%255s",host)!=1)return 1;
    memset(&hints,0,sizeof(hints));hints.ai_family=AF_UNSPEC;hints.ai_socktype=SOCK_STREAM;
    clock_gettime(CLOCK_MONOTONIC,&a);int rc=getaddrinfo(host,NULL,&hints,&res);clock_gettime(CLOCK_MONOTONIC,&b);
    double ms=(b.tv_sec-a.tv_sec)*1000.0+(b.tv_nsec-a.tv_nsec)/1e6;
    if(rc!=0){printf("Resolution failed: %s\n",gai_strerror(rc));printf("Elapsed: %.3f ms\n",ms);return 1;}
    puts("Resolved addresses:");
    for(p=res;p;p=p->ai_next){char ip[INET6_ADDRSTRLEN];void *addr;
        if(p->ai_family==AF_INET)addr=&((struct sockaddr_in*)p->ai_addr)->sin_addr;
        else if(p->ai_family==AF_INET6)addr=&((struct sockaddr_in6*)p->ai_addr)->sin6_addr;else continue;
        if(inet_ntop(p->ai_family,addr,ip,sizeof(ip)))puts(ip);}
    printf("DNS/name resolution elapsed time: %.3f ms\n",ms);
    puts("Note: system resolver cache or hosts file may affect the result.");freeaddrinfo(res);return 0;
}
