#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PORT 5001
int main(void) {
    int fd=socket(AF_INET,SOCK_DGRAM,0); struct sockaddr_in addr,peer;
    socklen_t len=sizeof(peer); char buf[4096];
    memset(&addr,0,sizeof(addr)); addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK); addr.sin_port=htons(PORT);
    if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))<0){perror("bind");return 1;}
    printf("UDP server listening on 127.0.0.1:%d\n",PORT);
    for(;;){
        ssize_t n=recvfrom(fd,buf,sizeof(buf)-1,0,(struct sockaddr*)&peer,&len);
        if(n<0){perror("recvfrom");break;} buf[n]='\0';
        if(strcmp(buf,"quit")==0){sendto(fd,"bye",3,0,(struct sockaddr*)&peer,len);break;}
        printf("Received: %s\n",buf);
        char reply[4200];snprintf(reply,sizeof(reply),"ACK:%s",buf);
        sendto(fd,reply,strlen(reply),0,(struct sockaddr*)&peer,len);
    }
    close(fd);return 0;
}
