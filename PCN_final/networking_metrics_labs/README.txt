NETWORKING LABS: RETRANSMISSION, CHECKSUM, ARP, DNS
===================================================

This folder contains simple Linux/Kali examples and their metrics.
These are educational demos, not full protocol-stack implementations.

FILES
-----
udp_retransmission_server.c
udp_retransmission_client.c
checksum_demo.c
arp_lookup.sh
dns_lookup.c

1) UDP RETRANSMISSION DEMO
--------------------------
Compile:
  gcc udp_retransmission_server.c -o udp_server
  gcc udp_retransmission_client.c -o udp_client

Run in two terminals:
  Terminal 1: ./udp_server
  Terminal 2: ./udp_client

The client sends sequence-numbered UDP packets and waits for ACKs.
If no ACK arrives within one second, it retransmits (up to 4 retries).

Metrics printed:
- original packets and ACKed packets
- retransmission count
- payload bytes sent including retries
- ACK bytes received
- packet delivery ratio
- average and maximum RTT
- elapsed time and sender throughput

This is an application-level reliability demo over UDP, not TCP retransmission.

2) CHECKSUM DEMO
----------------
Compile and run:
  gcc checksum_demo.c -o checksum_demo
  ./checksum_demo

Calculates a 16-bit Internet-style checksum, verifies data plus checksum,
then flips one bit to demonstrate error detection.

Metrics/output:
- timestamp
- message length
- checksum value
- verification result before and after corruption

Note: This demonstrates checksum logic; it is not a packet capture or a
complete TCP/UDP/IP checksum implementation with pseudo-headers.

3) ARP LOOKUP
------------
Make executable:
  chmod +x arp_lookup.sh

Run with an IPv4 address on your own LAN:
  ./arp_lookup.sh 192.168.1.1

Find your gateway IP with:
  ip route

Metrics/output:
- timestamp
- target IPv4 address
- neighbour/ARP cache state
- MAC address when available

ARP is limited to the local network. Use an address you are authorized to
inspect. On Linux, `ip neigh` is the standard way to view the neighbour cache.
The ping only encourages cache resolution; a blocked ping does not always
mean ARP failed.

4) DNS LOOKUP
-------------
Compile:
  gcc dns_lookup.c -o dns_lookup

Run:
  ./dns_lookup example.com

Or:
  ./dns_lookup google.com

Metrics/output:
- start timestamp
- resolved IPv4/IPv6 addresses
- approximate elapsed wall-clock time (whole-second resolution)
- number of addresses returned

This uses getaddrinfo(), which may use local caches and the operating
system's configured DNS resolver. It does not manually construct DNS packets.

GENERAL NOTES
-------------
- Compile on Linux/Kali with gcc.
- Timestamps use time()/ctime() for simplicity.
- RTT in the retransmission demo uses clock_gettime(CLOCK_MONOTONIC).
- For accurate DNS timing below one second, use clock_gettime() around
  getaddrinfo(); this simple example intentionally keeps the code beginner-friendly.
- On Windows, these POSIX examples need changes.
