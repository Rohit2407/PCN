BASIC TCP/UDP SOCKET PROGRAMS WITH TIMESTAMPS AND METRICS
=========================================================

Files:
  tcp_oneway_server.c / tcp_oneway_client.c
  tcp_twoway_server.c / tcp_twoway_client.c
  udp_oneway_server.c / udp_oneway_client.c
  udp_twoway_server.c / udp_twoway_client.c

These examples target Linux (including Kali Linux) and use POSIX sockets.

COMPILE (run in this folder):
  gcc tcp_oneway_server.c -o tcp_oneway_server
  gcc tcp_oneway_client.c -o tcp_oneway_client
  gcc tcp_twoway_server.c -o tcp_twoway_server
  gcc tcp_twoway_client.c -o tcp_twoway_client
  gcc udp_oneway_server.c -o udp_oneway_server
  gcc udp_oneway_client.c -o udp_oneway_client
  gcc udp_twoway_server.c -o udp_twoway_server
  gcc udp_twoway_client.c -o udp_twoway_client

RUN EACH EXPERIMENT:
Open two terminals. Start the server first, then run the matching client.
Example:
  Terminal 1: ./tcp_oneway_server
  Terminal 2: ./tcp_oneway_client

Ports:
  TCP one-way: 8081
  TCP two-way: 8082
  UDP one-way: 8083
  UDP two-way: 8084

The clients use 127.0.0.1 (same computer). For two different computers,
change 127.0.0.1 in the client to the server computer's IP address.

METRICS:
- Send/receive timestamps: human-readable wall-clock time.
- RTT: measured by two-way clients with a monotonic clock, from before send
  until the echo reply is received.
- Throughput: payload bytes divided by elapsed receive/transfer time.
- Byte and message/packet counts are printed.

IMPORTANT LIMITATIONS:
- TCP is a byte stream. One recv() is not guaranteed to equal one send().
  These introductory examples print received chunks; they are not a framing
  protocol. For larger or production messages, add message delimiters or
  length-prefix framing and loop for complete reads/writes.
- UDP one-way does not measure RTT because there is no reply. True one-way
  latency needs synchronized clocks and a send timestamp in the packet.
- The examples use local loopback. RTT/throughput results mostly reflect the
  local machine and are not representative of a real network.
- UDP two-way server/client use a fixed count of 10 packets for the lab demo.
- Metrics are simple lab estimates, not a full network benchmark.
