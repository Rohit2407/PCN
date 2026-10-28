// Minimal local HTTP server for one file. Usage: ./http_server file.txt [port]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
int main(int argc,char **argv){
    if(argc<2){printf("Usage: %s filename [port]\n",argv[0]);return 1;}
    int port=argc>2?atoi(argv[2]):8000;FILE *f=fopen(argv[1],"rb");
    if(!f){perror("fopen");return 1;}fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    int s=socket(AF_INET,SOCK_STREAM,0),opt=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    struct sockaddr_in a;memset(&a,0,sizeof(a));a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);a.sin_port=htons(port);
    if(bind(s,(struct sockaddr*)&a,sizeof(a))<0||listen(s,1)<0){perror("bind/listen");return 1;}
    printf("Serving %s at http://127.0.0.1:%d/ (one request)\n",argv[1],port);
    int c=accept(s,NULL,NULL);char req[2048];recv(c,req,sizeof(req)-1,0);
    char header[256];snprintf(header,sizeof(header),"HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: application/octet-stream\r\nConnection: close\r\n\r\n",size);
    send(c,header,strlen(header),0);char buf[8192];size_t n;while((n=fread(buf,1,sizeof(buf),f))>0)send(c,buf,n,0);
    close(c);close(s);fclose(f);return 0;
}
