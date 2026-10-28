// Run ./udp_retransmission server in one terminal, then ./udp_retransmission client.
// Stop-and-wait UDP example: sequence numbers, timeout and retransmission.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PORT 5010
int main(int argc,char **argv) {
    if(argc<2){printf("Usage: %s server|client\n",argv[0]);return 1;}
    int fd=socket(AF_INET,SOCK_DGRAM,0);struct sockaddr_in addr,peer;
    memset(&addr,0,sizeof(addr));addr.sin_family=AF_INET;addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);addr.sin_port=htons(PORT);
    if(strcmp(argv[1],"server")==0){
        if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))<0){perror("bind");return 1;}
        printf("UDP ACK server on port %d\n",PORT);
        for(;;){char buf[128];socklen_t l=sizeof(peer);int n=recvfrom(fd,buf,sizeof(buf)-1,0,(struct sockaddr*)&peer,&l);
            if(n<0)continue;buf[n]='\0';if(strcmp(buf,"STOP")==0)break;sendto(fd,buf,n,0,(struct sockaddr*)&peer,l);}
    } else {
        struct timeval tv={0,500000};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
        int logical=10,acked=0,transmissions=0,retries=0;double rttSum=0;
        for(int i=0;i<logical;i++){char msg[64],reply[128];snprintf(msg,sizeof(msg),"SEQ:%d",i);int ok=0;
            for(int attempt=0;attempt<=3;attempt++){struct timespec a,b;clock_gettime(CLOCK_MONOTONIC,&a);
                sendto(fd,msg,strlen(msg),0,(struct sockaddr*)&addr,sizeof(addr));transmissions++;
                socklen_t l=sizeof(peer);int n=recvfrom(fd,reply,sizeof(reply)-1,0,(struct sockaddr*)&peer,&l);
                if(n>0){reply[n]='\0';if(strcmp(reply,msg)==0){clock_gettime(CLOCK_MONOTONIC,&b);
                    rttSum+=(b.tv_sec-a.tv_sec)+(b.tv_nsec-a.tv_nsec)/1e9;acked++;retries+=attempt;ok=1;break;}}
            }
            printf("Sequence %d: %s\n",i,ok?"ACK received":"FAILED");
        }
        printf("Logical packets=%d, transmissions=%d, ACKed=%d\n",logical,transmissions,acked);
        printf("Retries=%d, PDR=%.2f%%, average RTT=%.3f ms\n",retries,100.0*acked/logical,acked?rttSum*1000/acked:0);
        printf("Approx effective goodput=%.6f Mbps\n",rttSum>0?acked*8.0*sizeof("SEQ:0000")/rttSum/1e6:0);
        sendto(fd,"STOP",4,0,(struct sockaddr*)&addr,sizeof(addr));
    }
    close(fd);return 0;
}
