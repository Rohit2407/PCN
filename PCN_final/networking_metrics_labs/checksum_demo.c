#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

unsigned short checksum(const unsigned char data[], int len)
{
    unsigned long sum = 0;
    int i;

    for (i = 0; i + 1 < len; i += 2) {
        unsigned short word = ((unsigned short)data[i] << 8) | data[i + 1];
        sum += word;
    }

    if (len % 2)
        sum += (unsigned short)data[len - 1] << 8;

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (unsigned short)(~sum);
}

int main(void)
{
    unsigned char message[] = "Networking checksum example";
    int len = strlen((char *)message);
    unsigned short csum = checksum(message, len);

    time_t now = time(NULL);
    printf("Timestamp: %s", ctime(&now));
    printf("Message: %s\n", message);
    printf("Message length: %d bytes\n", len);
    printf("Internet checksum: 0x%04X\n", csum);

    unsigned char packet[128];
    memcpy(packet, message, len);
    packet[len] = (csum >> 8) & 0xFF;
    packet[len + 1] = csum & 0xFF;

    unsigned short verify = checksum(packet, len + 2);
    printf("Verification checksum over data + checksum: 0x%04X\n", verify);
    if (verify == 0)
        printf("Result: checksum valid (no detected corruption)\n");
    else
        printf("Result: checksum mismatch / corruption detected\n");

    packet[0] ^= 1; /* deliberately flip one bit */
    verify = checksum(packet, len + 2);
    printf("After flipping one bit, verification: 0x%04X\n", verify);
    if (verify == 0)
        printf("Result: checksum still valid\n");
    else
        printf("Result: corruption detected\n");

    return 0;
}
