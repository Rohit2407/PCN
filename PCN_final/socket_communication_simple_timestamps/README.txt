BASIC TCP AND UDP COMMUNICATION WITH SIMPLE TIMESTAMPS + METRICS
================================================================

Contains 8 C programs:
  tcp_oneway_server.c     tcp_oneway_client.c
  tcp_twoway_server.c     tcp_twoway_client.c
  udp_oneway_server.c     udp_oneway_client.c
  udp_twoway_server.c     udp_twoway_client.c

Compile on Linux/Kali:
  gcc tcp_oneway_server.c -o tcp_oneway_server
  gcc tcp_oneway_client.c -o tcp_oneway_client
  gcc tcp_twoway_server.c -o tcp_twoway_server
  gcc tcp_twoway_client.c -o tcp_twoway_client
  gcc udp_oneway_server.c -o udp_oneway_server
  gcc udp_oneway_client.c -o udp_oneway_client
  gcc udp_twoway_server.c -o udp_twoway_server
  gcc udp_twoway_client.c -o udp_twoway_client

For each experiment, open two terminals and run the server first.
Example:
  Terminal 1: ./tcp_oneway_server
  Terminal 2: ./tcp_oneway_client

Ports:
  TCP one-way 8081
  TCP two-way 8082
  UDP one-way 8083
  UDP two-way 8084

The clients use 127.0.0.1 (same machine). For separate machines, replace
127.0.0.1 in the client with the server's IP address.

Timestamp functions:
  time(NULL) + ctime(&t) gives an easy readable timestamp (about one-second
  precision).
  clock_gettime(CLOCK_REALTIME, &ts) displays seconds and nanoseconds since
  the Unix epoch, useful for showing a finer-grained timestamp.
  clock_gettime(CLOCK_MONOTONIC, ...) is used for elapsed time and RTT,
  because wall-clock time can jump if the system clock changes.

Metrics:
  - send/receive timestamps
  - RTT in two-way client programs
  - payload bytes and counts
  - basic throughput
  - total elapsed duration

Notes:
  - One-way programs have no reply, so they do not calculate RTT.
  - True one-way network latency needs synchronized clocks and a timestamp
    sent in the packet. These local examples do not measure true one-way
    network latency.
  - TCP is a byte stream: one recv() is not guaranteed to correspond to one
    send(). These short examples are for basic lab learning.
  - The UDP two-way server echoes 10 packets and then exits. One-way UDP
    server likewise expects 10 packets.
  - The programs include basic error checks for socket, bind, listen,
    accept, connect, send, recv, sendto and recvfrom where applicable.
