#include <stdio.h>
#include <string.h>
#include <time.h>
#define MAX 2048
void divide(char bits[],const char *gen,char rem[]){
    char work[MAX];strncpy(work,bits,MAX-1);work[MAX-1]='\0';int n=strlen(gen),len=strlen(work);
    for(int i=0;i<=len-n;i++)if(work[i]=='1')for(int j=0;j<n;j++)work[i+j]=(work[i+j]==gen[j])?'0':'1';
    strncpy(rem,work+len-(n-1),n-1);rem[n-1]='\0';
}
int main(void){
    char data[MAX],gen[MAX],app[MAX],rem[MAX],code[MAX],check[MAX];clock_t start=clock();
    printf("Enter data bits (e.g. 110101): ");scanf("%2047s",data);
    printf("Enter generator bits (e.g. 10011): ");scanf("%2047s",gen);
    int n=strlen(gen),d=strlen(data);
    if(n<2||gen[0]!='1'||d+n>=MAX){puts("Invalid input");return 1;}
    for(int i=0;i<d;i++)if(data[i]!='0'&&data[i]!='1'){puts("Bits only");return 1;}
    for(int i=0;i<n;i++)if(gen[i]!='0'&&gen[i]!='1'){puts("Bits only");return 1;}
    strcpy(app,data);for(int i=0;i<n-1;i++)app[d+i]='0';app[d+n-1]='\0';
    divide(app,gen,rem);strcpy(code,data);strcat(code,rem);
    printf("CRC remainder: %s\nTransmitted codeword: %s\n",rem,code);
    divide(code,gen,check);printf("Receiver remainder: %s\nCRC check: %s\n",check,strspn(check,"0")==strlen(check)?"NO ERROR DETECTED":"ERROR DETECTED");
    code[strlen(code)-1]=code[strlen(code)-1]=='0'?'1':'0';divide(code,gen,check);
    printf("After flipping last bit: %s\nCheck: %s\n",code,strspn(check,"0")==strlen(check)?"NO ERROR DETECTED":"ERROR DETECTED");
    printf("Execution time: %.3f ms\n",1000.0*(clock()-start)/CLOCKS_PER_SEC);return 0;
}
