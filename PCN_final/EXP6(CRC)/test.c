#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 8080
int recieve_all(int sock,void * data,int size)
{
    int recieved=0;
    while (recieved<size){
        int n=recv(sock,(char *)data+recieved,size-recieved,0)
        if (n<=0){
            return 0;
        }
        recieved+=n;
    }
    return 1;
}
int main(){
    int server,client,length;
    struct sockaddr_in addr;
    char frame[8192];
    char temp[8192];
    char generator[]='10011';
    server=socket(AF_i)
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(PORT);
    addr.sin_addr.s_addr=INADDR_ANY;

    bind(server,(struct sockaddr*)&addr,sizeof(addr));
    listen(server,5);
    client=accept(server,null,null);
    receive_all(client,&length,sizeof(length),0);
    receive_all(client,frame,length,0);
    frame[length]='\0'
    frame(frame[0]=='0')?'1':'0';
    strcpy(temp,frame);
    for (int i=0;i<=length-5;i++){
        if temp[i]==1{
            for (int j=0;j<length-5;j++){
                temp[i+j]=(temp[i+j]==generator[j])?'0':'1';
            }
        }
    }
}