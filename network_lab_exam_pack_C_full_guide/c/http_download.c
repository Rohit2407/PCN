// Usage: ./http_download 127.0.0.1 8000 / filename.out
// Download path is sent as the HTTP request path. Intended for a local lab server.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
int main(int argc,char **argv){
    if(argc<5){printf("Usage: %s host port request_path output_file\n",argv[0]);return 1;}
    const char *host=argv[1],*path=argv[3],*outname=argv[4];int port=atoi(argv[2]);
    int s=socket(AF_INET,SOCK_STREAM,0);struct sockaddr_in a;memset(&a,0,sizeof(a));
    a.sin_family=AF_INET;a.sin_port=htons(port);
    if(inet_pton(AF_INET,host,&a.sin_addr)!=1){puts("Use an IPv4 address such as 127.0.0.1");return 1;}
    if(connect(s,(struct sockaddr*)&a,sizeof(a))<0){perror("connect");return 1;}
    char req[2048];snprintf(req,sizeof(req),"GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",path,host);
    struct timespec start,end;clock_gettime(CLOCK_MONOTONIC,&start);send(s,req,strlen(req),0);
    FILE *out=fopen(outname,"wb");if(!out){perror("fopen");return 1;}
    char buf[8192],header[8192];size_t headerlen=0;int found=0;long long bytes=0;ssize_t n;
    while((n=recv(s,buf,sizeof(buf),0))>0){
        if(!found){for(ssize_t i=0;i<n;i++){header[headerlen++]=buf[i];
            if(headerlen>=4&&memcmp(header+headerlen-4,"\r\n\r\n",4)==0){found=1;
                fwrite(buf+i+1,1,(size_t)(n-i-1),out);bytes+=n-i-1;break;}
            if(headerlen>=sizeof(header)-1){puts("HTTP header too large");return 1;}}
        }else{fwrite(buf,1,(size_t)n,out);bytes+=n;}
    }
    clock_gettime(CLOCK_MONOTONIC,&end);double sec=(end.tv_sec-start.tv_sec)+(end.tv_nsec-start.tv_nsec)/1e9;
    fclose(out);close(s);
    printf("HTTP response: %.40s\n",header);
    printf("Downloaded body bytes: %lld\nElapsed time: %.4f s\nThroughput: %.4f Mbps\n",bytes,sec,sec>0?bytes*8.0/sec/1e6:0);
    printf("Saved to: %s\n",outname);return 0;
}
