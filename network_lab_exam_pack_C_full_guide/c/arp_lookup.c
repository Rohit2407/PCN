#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int main(void){
    clock_t start=clock();
    puts("ARP/neighbour cache (Linux):");
    int rc=system("ip neigh show");
    if(rc!=0){puts("Trying fallback command: arp -a");system("arp -a");}
    printf("Command execution time: %.3f ms\n",1000.0*(clock()-start)/CLOCKS_PER_SEC);
    puts("Note: entries appear after communication on the local network; this reads the OS cache.");
    return 0;
}
