#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
uint16_t make_checksum(const unsigned char *data,size_t len){
    uint32_t sum=0;for(size_t i=0;i<len;i+=2){uint16_t word=(uint16_t)data[i]<<8;
        if(i+1<len)word|=data[i+1];sum+=word;sum=(sum&0xffff)+(sum>>16);}
    while(sum>>16)sum=(sum&0xffff)+(sum>>16);return (uint16_t)(~sum);
}
int main(void){
    char msg[1024];printf("Enter message: ");if(!fgets(msg,sizeof(msg),stdin))return 1;
    size_t len=strlen(msg);if(len&&msg[len-1]=='\n')msg[--len]='\0';
    clock_t start=clock();uint16_t cs=make_checksum((unsigned char*)msg,len);
    printf("Checksum: 0x%04X\n",cs);
    unsigned char packet[1026];memcpy(packet,msg,len);packet[len]=cs>>8;packet[len+1]=cs&255;
    size_t plen=len+2;uint32_t sum=0;
    for(size_t i=0;i<plen;i+=2){uint16_t w=(uint16_t)packet[i]<<8;if(i+1<plen)w|=packet[i+1];sum+=w;sum=(sum&0xffff)+(sum>>16);}
    while(sum>>16)sum=(sum&0xffff)+(sum>>16);
    printf("Receiver one's-complement sum: 0x%04X\nValidation: %s\n",sum,sum==0xffff?"VALID":"INVALID");
    printf("Execution time: %.3f ms\n",1000.0*(clock()-start)/CLOCKS_PER_SEC);return 0;
}
