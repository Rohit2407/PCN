NS-3 TCP VARIANT COMPARISON (SIX NODES)
======================================
One C++ file compares TCP NewReno, CUBIC and BBR by running three separate simulations.
IMPORTANT: In modern NS-3, TcpNewReno is the Reno-family implementation; Reno and
NewReno are not necessarily two independent algorithms available in your build.

INSTALL/RUN (NS-3.42-style):
1. Copy tcp_compare.cc to your NS-3 folder's scratch directory.
2. From NS-3 root:
   ./ns3 run scratch/tcp_compare
3. Change the values supplied by the question:
   ./ns3 run "scratch/tcp_compare --bw=10Mbps --delay=5ms --packetSize=512 --stopTime=20"

DEFAULT TOPOLOGY (6 nodes):
     1 --- 2
    /       \
   0         5
    \       /
     3 --- 4
Source: node 0; destination: node 5.

TO CHANGE WIRED TOPOLOGY:
Find the `edges` list in tcp_compare.cc. Every {a,b} creates a point-to-point link.
Examples (all with nodes numbered 0 through 5):
STAR:     {{0,1},{0,2},{0,3},{0,4},{0,5}}
LINEAR:   {{0,1},{1,2},{2,3},{3,4},{4,5}}
RING:     {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0}}
TREE:     {{0,1},{0,2},{1,3},{1,4},{2,5}}
TWO PATHS:{{0,1},{1,2},{2,5},{0,3},{3,4},{4,5}}
For a different node count, change nodes.Create(6), edges, and source/destination
node indexes. Ensure node 5 has at least one link: its interface 1 is used as destination.

METRICS (FlowMonitor):
- Throughput Mbps: received IP bytes * 8 / (last receive - first transmit) / 1e6
- Average one-way IP delay: delaySum / rxPackets (ms)
- Average jitter: jitterSum / (rxPackets-1) (ms)
- PDR: rxPackets / txPackets * 100
- Loss: (txPackets-rxPackets)/txPackets * 100
- Tx and Rx packet counts

IMPORTANT LIMITATIONS:
- TCP is a reliable byte stream, so FlowMonitor's IP-packet loss and PDR are NOT
  application delivery success rates; retransmitted segments complicate counts.
- `lostPackets` is also available but is not a direct TCP retransmission counter.
- RTT, congestion window, fairness, and exact TCP retransmissions are NOT
  measured by this simple file. Do not claim otherwise in an exam report.
- Signal strength is NOT applicable to this wired example.
- BBR may be absent in older NS-3 versions; the program prints unavailable.
- Source has NOT been compiled against a local NS-3 installation here.
