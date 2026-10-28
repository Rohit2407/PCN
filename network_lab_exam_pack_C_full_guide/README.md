# NETWORK LAB EXAM PACK — FULL RUN GUIDE
**C for socket-programming/fundamentals programs. NS-3 simulation files use C++ (NS-3 is a C++ framework).**

This guide explains what each file does, how to compile/run it, how to change the parameters, and how to change a topology to match a question diagram.

---

# 1. First identify which kind of program the question asks for

| If the question says… | Use this file |
|---|---|
| NS-3, six nodes, wired links, TCP vs UDP, bandwidth/delay/queue | `ns3/wired_tcp_udp.cc` |
| Compare three TCP congestion-control algorithms | `ns3/tcp_variants.cc` |
| NS-3 wireless/Wi-Fi nodes | `ns3/wireless_wifi.cc` |
| TCP client and server using sockets | `c/tcp_server.c`, `c/tcp_client.c` |
| UDP client and server using sockets | `c/udp_server.c`, `c/udp_client.c` |
| UDP timeout, sequence number, retransmission | `c/udp_retransmission.c` |
| TCP bulk transfer and goodput | `c/tcp_transfer_metrics.c` |
| CRC generator, remainder, receiver verification | `c/crc_demo.c` |
| Internet checksum and verification | `c/checksum_demo.c` |
| ARP table / IP-to-MAC neighbour cache | `c/arp_lookup.c` |
| DNS hostname lookup | `c/dns_lookup.c` |
| Download a page/file from a local HTTP server | `c/http_server.c`, `c/http_download.c` |

## Metrics vocabulary
- **Throughput:** received bits per second.
- **Goodput:** useful application data delivered per second.
- **RTT:** time from sending a request until its response arrives.
- **Average delay:** average packet receive time minus packet send time.
- **PDR:** received packets / sent packets × 100%.
- **Packet loss:** sent packets − received packets.
- **Retransmission:** sending a packet again after a timeout or detected loss.
- **CRC/checksum:** error-detection methods; neither encrypts data.
- **DNS lookup time:** elapsed time for the system's hostname resolution call; caches may affect it.
- **HTTP download time/rate:** time and payload rate for the downloaded response.

---

# 2. NS-3 setup and run commands

## Copy files
Open a terminal in your NS-3 installation directory. Copy the desired `.cc` file into `scratch/`. For example, if the ZIP was extracted into `~/Downloads/network_lab_exam_pack_c`:

```bash
cp ~/Downloads/network_lab_exam_pack_c/ns3/wired_tcp_udp.cc scratch/
cp ~/Downloads/network_lab_exam_pack_c/ns3/tcp_variants.cc scratch/
cp ~/Downloads/network_lab_exam_pack_c/ns3/wireless_wifi.cc scratch/
```

Run one program at a time. Modern NS-3:
```bash
./ns3 run scratch/wired_tcp_udp
```
Older NS-3 installations may use:
```bash
./waf --run scratch/wired_tcp_udp
```

## A. Wired TCP and UDP experiment
Default values in the program:
- 6 nodes
- 5 point-to-point links
- line topology: Node 0 -- Node 1 -- Node 2 -- Node 3 -- Node 4 -- Node 5
- link data rate: 5 Mbps
- link delay: 5 ms per link
- queue: DropTail, 50 packets
- UDP packet size: 1024 bytes
- UDP offered rate: 2 Mbps
- simulation duration: 30 seconds

Run with defaults:
```bash
./ns3 run scratch/wired_tcp_udp
```

Change the values to match the question/README:
```bash
./ns3 run "scratch/wired_tcp_udp --rate=10Mbps --delay=2ms --simTime=40 --packetSize=512 --queuePackets=100 --appRate=1"
```

What the arguments mean:
- `--rate`: capacity of every point-to-point link.
- `--delay`: propagation delay on each point-to-point link.
- `--simTime`: total simulation time in seconds.
- `--packetSize`: UDP application packet size in bytes.
- `--queuePackets`: DropTail queue capacity in packets.
- `--appRate`: offered UDP application rate. Use values such as `1Mbps` or `500Kbps` only if supported by the current code's parser; the wired program's `appRate` argument is numeric Mbps, so use `--appRate=1` for 1 Mbps, `--appRate=0.5` for 0.5 Mbps.

The code prints flow-level transmitted/received packet counts, approximate loss, PDR, received bytes, throughput, and average delay. TCP and UDP are different flows, so compare their rows.

## B. TCP congestion-control comparison (three algorithms)
The `tcp_variants.cc` program uses the same six-node wired line and runs one variant per invocation. Keep all other parameters the same between runs.

```bash
./ns3 run "scratch/tcp_variants --variant=Reno"
./ns3 run "scratch/tcp_variants --variant=NewReno"
./ns3 run "scratch/tcp_variants --variant=Cubic"
```

You can also change link rate, delay, duration, and packet size:
```bash
./ns3 run "scratch/tcp_variants --variant=NewReno --rate=10Mbps --delay=3ms --simTime=30 --packetSize=1400"
```

Record the output for all three runs in a comparison table. Do not change topology/rate/delay for just one variant. If a variant name is unsupported by your installed NS-3 release, the run will report a TypeId error; use a TCP variant supported by that release. **BBR is not configured as one of these three and is not available in every NS-3 version.**

## C. Wireless Wi-Fi experiment
Run with defaults:
```bash
./ns3 run scratch/wireless_wifi
```
Change parameters:
```bash
./ns3 run "scratch/wireless_wifi --nodes=6 --distance=10 --simTime=30 --packetSize=512 --appRate=1Mbps"
```
- `--nodes`: number of Wi-Fi nodes.
- `--distance`: spacing in metres between adjacent node positions along a line.
- `--simTime`: simulation duration in seconds.
- `--packetSize`: UDP packet size in bytes.
- `--appRate`: offered application rate.

The wireless program places nodes along a line. The wireless signal can travel directly between nodes if they are within radio range; the positions are not wired links. To change the layout, edit the `positions->Add(Vector(x, y, 0.0));` lines in the source.

---

# 3. How to change the NS-3 topology / routes

## Topology example 1 — six nodes in a line (default wired program)
```text
Node 0 ---- Node 1 ---- Node 2 ---- Node 3 ---- Node 4 ---- Node 5
 Source                                                     Destination
                         TCP and UDP flows
```
There are five point-to-point links. Each link gets its own IP subnet. In `wired_tcp_udp.cc`, find this loop:
```cpp
for (uint32_t i = 0; i < 5; ++i)
{
    NetDeviceContainer devices = p2p.Install(nodes.Get(i), nodes.Get(i + 1));
    // assign a different subnet to this link
}
```
This loop creates the line by connecting node `i` to node `i+1`.

## Topology example 2 — router in the middle with branches
Example diagram:
```text
Node 0 ----\
            Router (Node 2) ---- Node 3 ---- Node 5
Node 1 ----/          |
                      |
                    Node 4
```
To build this topology, replace the line-topology loop with explicit links. For example:
```cpp
NetDeviceContainer d02 = p2p.Install(nodes.Get(0), nodes.Get(2));
NetDeviceContainer d12 = p2p.Install(nodes.Get(1), nodes.Get(2));
NetDeviceContainer d23 = p2p.Install(nodes.Get(2), nodes.Get(3));
NetDeviceContainer d24 = p2p.Install(nodes.Get(2), nodes.Get(4));
NetDeviceContainer d35 = p2p.Install(nodes.Get(3), nodes.Get(5));
```
**Important:** this example has six nodes, with Node 2 acting as a router. You must assign a different IPv4 subnet to every point-to-point link. Example subnet plan:
- Node 0 ↔ Node 2: `10.1.1.0/24`
- Node 1 ↔ Node 2: `10.1.2.0/24`
- Node 2 ↔ Node 3: `10.1.3.0/24`
- Node 2 ↔ Node 4: `10.1.4.0/24`
- Node 3 ↔ Node 5: `10.1.5.0/24`

For each `NetDeviceContainer`, use a fresh `Ipv4AddressHelper`, call `SetBase()` with the matching subnet, and call `Assign()` to obtain the two endpoint addresses. Save the destination-side IP address for your application sink/client destination.

Then install the Internet stack on all nodes and populate routes:
```cpp
InternetStackHelper internet;
internet.Install(nodes);
Ipv4GlobalRoutingHelper::PopulateRoutingTables();
```
Call `PopulateRoutingTables()` after all links and IP addresses are installed. This computes routes for reachable IP networks. If your assignment explicitly requires a routing protocol such as RIP/OSPF/AODV, global static routing is not the same thing; use the protocol required by the question and your installed NS-3 modules.

## How to change the source and destination
In the app setup, the source is the node where the client/source application is installed:
```cpp
clientApps = client.Install(nodes.Get(0)); // source Node 0
```
The destination is the node where the sink/server is installed:
```cpp
serverApps = server.Install(nodes.Get(5)); // destination Node 5
```
For a different source/destination, change these node indices and change the destination IP address to the address assigned to the destination node on the relevant link. In a multi-link topology, do not guess the destination IP; use the saved `Ipv4InterfaceContainer` address or print it.

## How to change link settings independently
The starter code applies one rate and delay to all links. If the question gives different values per link, create/configure a separate `PointToPointHelper` per link, for example:
```cpp
PointToPointHelper fastLink;
fastLink.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
fastLink.SetChannelAttribute("Delay", StringValue("2ms"));

PointToPointHelper slowLink;
slowLink.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
slowLink.SetChannelAttribute("Delay", StringValue("10ms"));
```
Use `fastLink.Install(...)` or `slowLink.Install(...)` on the appropriate pair of nodes. Set the queue attribute on each helper if queue sizes differ.

---

# 4. C socket programs — compile and run

These C programs use POSIX sockets and are intended for Linux/Kali/WSL. Run the server and client in two terminals.

## A. TCP client/server
Terminal 1:
```bash
gcc c/tcp_server.c -o tcp_server
./tcp_server
```
Terminal 2:
```bash
gcc c/tcp_client.c -o tcp_client
./tcp_client
```
What happens: TCP establishes a connection, the client sends a message, the server replies with `ACK:...`. The client reports approximate request/response time and reply throughput. TCP is a byte stream and the OS TCP stack handles retransmissions.

## B. UDP client/server
Terminal 1:
```bash
gcc c/udp_server.c -o udp_server
./udp_server
```
Terminal 2:
```bash
gcc c/udp_client.c -o udp_client
./udp_client
```
What happens: UDP sends one datagram and the server replies. The client has a 2-second receive timeout. The one-packet PDR is either 100% if the reply arrives or 0% if it times out.

## C. UDP timer/retransmission
Terminal 1:
```bash
gcc c/udp_retransmission.c -o udp_retransmission
./udp_retransmission server
```
Terminal 2:
```bash
gcc c/udp_retransmission.c -o udp_retransmission
./udp_retransmission client
```
What happens: the client sends sequence-numbered datagrams, waits up to 0.5 seconds for an ACK, retries up to three times, then prints transmissions, retries, PDR and average RTT.

## D. TCP bulk transfer
Terminal 1:
```bash
gcc c/tcp_transfer_metrics.c -o tcp_transfer_metrics
./tcp_transfer_metrics server
```
Terminal 2:
```bash
gcc c/tcp_transfer_metrics.c -o tcp_transfer_metrics
./tcp_transfer_metrics client 1000000
```
The final argument is the number of bytes to transfer. Output includes receiver-confirmed bytes, elapsed time and goodput.

---

# 5. CRC, checksum, ARP, DNS — compile and run

## CRC
```bash
gcc c/crc_demo.c -o crc_demo
./crc_demo
```
Enter data bits, e.g. `110101`, then generator bits, e.g. `10011`. It prints the CRC remainder and transmitted codeword, checks the receiver remainder, flips the last bit and tests again, and reports execution time.

## Checksum
```bash
gcc c/checksum_demo.c -o checksum_demo
./checksum_demo
```
Enter a message. It prints the checksum, receiver validation and execution time.

## ARP / neighbour table
```bash
gcc c/arp_lookup.c -o arp_lookup
./arp_lookup
```
It displays the operating system's neighbour cache using `ip neigh show`, with `arp -a` as a fallback. An empty table is possible if the host has not recently communicated on the local LAN.

## DNS
```bash
gcc c/dns_lookup.c -o dns_lookup
./dns_lookup
```
Enter a hostname such as `example.com`. It prints resolved IPv4/IPv6 addresses and lookup elapsed time. The machine must have working DNS/network access for external hostnames.

---

# 6. Local HTTP file download in C

In the folder containing a file you want to serve, create a small file:
```bash
echo "Hello from local HTTP server" > sample.txt
```
Compile:
```bash
gcc c/http_server.c -o http_server
gcc c/http_download.c -o http_download
```
Terminal 1 — serve the file on localhost:
```bash
./http_server sample.txt 8000
```
Terminal 2 — download it:
```bash
./http_download 127.0.0.1 8000 / downloaded_sample.txt
```
The client prints HTTP response, downloaded body bytes, elapsed time and throughput. The sample server handles one request and then exits. Use `127.0.0.1` because both programs run on the same machine. To test across two lab machines, you would need to bind the server to an accessible interface and use the server's reachable IP; only do this on a network you are authorised to use.

---

# 7. Lab report / exam output table

Fill this table using your actual program output:

| Run | Tx packets | Rx packets | Loss | PDR (%) | Throughput (Mbps) | Avg delay (ms) | RTT (ms) |
|---|---:|---:|---:|---:|---:|---:|---:|
| NS-3 TCP | | | | | | | |
| NS-3 UDP | | | | | | | |
| TCP Reno | | | | | | | |
| TCP NewReno | | | | | | | |
| TCP Cubic | | | | | | | |
| NS-3 Wi-Fi | | | | | | | |

For C socket programs, report the metrics the program actually measures. Not every program has packet-level PDR because ordinary TCP sockets expose a byte stream rather than individual TCP segments.

## Common exam changes
1. **Six nodes to another count:** change `nodes.Create(...)` and update the link-creation code and source/destination indices.
2. **Line to branched graph:** replace the `i` to `i+1` loop with explicit `p2p.Install(nodes.Get(a), nodes.Get(b))` calls matching the drawing.
3. **Different link rates/delays:** use separate `PointToPointHelper` objects for links with different values.
4. **Different packet size/rate/duration:** use the command-line arguments shown above.
5. **Different source/destination:** update the client/source node index, server/sink node index, and destination IP.
6. **Metrics:** use FlowMonitor for NS-3 flow metrics; use timestamps/counters for C socket-program metrics.
7. **TCP variant comparison:** change only `--variant` between the three runs; keep everything else identical.

Always check the question's README for parameter values. These files are templates, so test them with the installed NS-3 release before the exam.
