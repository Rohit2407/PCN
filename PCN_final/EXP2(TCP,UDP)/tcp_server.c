
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define SIZE 1024

void handle_client(int client)
{
    char roll[50] = {0};
    char line[SIZE];
    char response[SIZE] = "Student Record Not Found";

    recv(client, roll, sizeof(roll) - 1, 0);

    FILE *fp = fopen("students.csv", "r");

    if (fp != NULL)
    {
        fgets(line, SIZE, fp);

        while (fgets(line, SIZE, fp))
        {
            char copy[SIZE];
            strcpy(copy, line);

            char *rno = strtok(copy, ",");

            if (rno != NULL && strcmp(rno, roll) == 0)
            {
                strcpy(response, line);
                break;
            }
        }

        fclose(fp);
    }
    else
    {
        strcpy(response, "Unable to open students.csv");
    }

    send(client, response, strlen(response) + 1, 0);

    close(client);
}

int main()
{
    int server, client;
    struct sockaddr_in addr;

    server = socket(AF_INET, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr *)&addr, sizeof(addr));
    listen(server, 5);

    printf("TCP Server running on port %d\n", PORT);

    while (1)
    {
        client = accept(server, NULL, NULL);
        handle_client(client);
    }

    close(server);
    return 0;
}
