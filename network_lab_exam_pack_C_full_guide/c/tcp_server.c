#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PORT 5000
int main(void) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0), client_fd;
    struct sockaddr_in addr;
    char buffer[4096] = {0};
    if (server_fd < 0) { perror("socket"); return 1; }
    int opt = 1; setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&addr, 0, sizeof(addr)); addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (listen(server_fd, 1) < 0) { perror("listen"); return 1; }
    printf("TCP server listening on 127.0.0.1:%d\n", PORT);
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) { perror("accept"); return 1; }
    ssize_t n = recv(client_fd, buffer, sizeof(buffer)-1, 0);
    if (n > 0) {
        buffer[n] = '\0'; printf("Received: %s\n", buffer);
        char reply[4200]; snprintf(reply, sizeof(reply), "ACK:%s", buffer);
        send(client_fd, reply, strlen(reply), 0);
    }
    close(client_fd); close(server_fd); return 0;
}
