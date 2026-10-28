#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    const char *hostname = (argc > 1) ? argv[1] : "example.com";
    struct addrinfo hints, *result, *p;
    int status;
    int count = 0;
    time_t t1, t2;

    time(&t1);
    printf("DNS lookup started at: %s", ctime(&t1));
    printf("Hostname: %s\n", hostname);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, NULL, &hints, &result);
    time(&t2);

    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    printf("\nResolved addresses:\n");
    for (p = result; p != NULL; p = p->ai_next) {
        char ip[INET6_ADDRSTRLEN];
        void *addr;

        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
        } else if (p->ai_family == AF_INET6) {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        } else {
            continue;
        }

        if (inet_ntop(p->ai_family, addr, ip, sizeof(ip)) != NULL) {
            printf("%s\n", ip);
            count++;
        }
    }

    printf("\n--- DNS LOOKUP METRICS ---\n");
    printf("Elapsed wall-clock time: %ld seconds (whole-second precision)\n", (long)(t2 - t1));
    printf("Address records returned: %d\n", count);
    printf("Lookup result: success\n");
    printf("Note: getaddrinfo may use the OS cache or configured resolver.\n");

    freeaddrinfo(result);
    return 0;
}
