
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int receive_all(int sock, void *data, int size)
{
    int received = 0;

    while (received < size)
    {
        int n = recv(sock, (char *)data + received,
                     size - received, 0);

        if (n <= 0)
            return 0;

        received += n;
    }

    return 1;
}

int main()
{
    int server, client, length;
    struct sockaddr_in addr;
    char frame[8192], temp[8192];
    char generator[] = "10011";

    server = socket(AF_INET, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr *)&addr, sizeof(addr));
    listen(server, 5);

    printf("CRC Server waiting...\n");

    client = accept(server, NULL, NULL);

    receive_all(client, &length, sizeof(length));
    receive_all(client, frame, length);

    frame[length] = '\0';

    printf("Received frame: %s\n", frame);

    // Introduce bit error
    frame[0] = (frame[0] == '0') ? '1' : '0';

    printf("Frame after error: %s\n", frame);

    strcpy(temp, frame);

    // CRC division
    for (int i = 0; i <= length - 5; i++)
    {
        if (temp[i] == '1')
        {
            for (int j = 0; j < 5; j++)
            {
                temp[i + j] =
                    (temp[i + j] == generator[j]) ? '0' : '1';
            }
        }
    }

    printf("CRC remainder: %s\n", temp + length - 4);

    if (strspn(temp + length - 4, "0") == 4)
        printf("Frame is ERROR-FREE\n");
    else
        printf("ERROR DETECTED\n");

    close(client);
    close(server);

    return 0;
}
