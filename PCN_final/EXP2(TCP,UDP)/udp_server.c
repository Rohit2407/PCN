
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);
    char roll[50], line[SIZE], response[SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr *)&server, sizeof(server));

    printf("UDP Server running on port %d\n", PORT);

    while (1)
    {
        memset(roll, 0, sizeof(roll));

        recvfrom(sock, roll, sizeof(roll) - 1, 0,
                 (struct sockaddr *)&client, &len);

        FILE *fp = fopen("students.csv", "r");
        strcpy(response, "Student Record Not Found");

        if (fp != NULL)
        {
            fgets(line, SIZE, fp);  // Skip header

            while (fgets(line, SIZE, fp))
            {
                char copy[SIZE];
                strcpy(copy, line);

                char *rno = strtok(copy, ",");
                char *name = strtok(NULL, ",");

                if (rno != NULL && strcmp(rno, roll) == 0)
                {
                    if (name != NULL)
                        sprintf(response, "Roll No: %s\nName: %s",
                                rno, name);
                    break;
                }
            }

            fclose(fp);
        }
        else
        {
            strcpy(response, "Unable to open students.csv");
        }

        sendto(sock, response, strlen(response) + 1, 0,
               (struct sockaddr *)&client, len);
    }

    close(sock);
    return 0;
}
