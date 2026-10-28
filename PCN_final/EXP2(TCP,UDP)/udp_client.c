
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in server;
    char roll[50], response[SIZE] = {0};

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Enter Roll Number: ");
    scanf("%49s", roll);

    sendto(sock, roll, strlen(roll), 0,
           (struct sockaddr *)&server, sizeof(server));

    recvfrom(sock, response, SIZE - 1, 0, NULL, NULL);

    printf("Server Response:\n%s\n", response);

    close(sock);
    return 0;
}
