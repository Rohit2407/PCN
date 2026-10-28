NS-3 WIRED + WIRELESS EXAM PACK (6 NODES DEFAULT)
================================================

IMPORTANT
---------
These examples use the NS-3.42 CMake wrapper:
    ./ns3 build
    ./ns3 run scratch/wired_exam
    ./ns3 run scratch/wireless_exam

Copy the .cc files into your NS-3 project's scratch folder. These are simple UDP examples.
They print throughput, packet delay, jitter, PDR, packet loss, packet/byte counts and an
estimated RTT (2 x one-way delay). That estimated RTT is NOT a measured TCP RTT.
Because these programs send UDP, TCP retransmissions are not applicable. If the question
specifically asks for TCP retransmissions, use a TCP experiment/template instead.

------------------------------------------------------------
1. WIRED: wired_exam.cc
------------------------------------------------------------

Copy:
    cp wired_exam.cc /path/to/ns-3/scratch/wired_exam.cc

From the NS-3 root:
    ./ns3 build
    ./ns3 run scratch/wired_exam

Change values from command line (no need to edit code for these):
    ./ns3 run "scratch/wired_exam --bw=10Mbps --delay=5ms --rate=2 --packetSize=1024 --simTime=15"

Arguments:
    --nodes=6         number of nodes (keep topology edges within this number)
    --bw=5Mbps        bandwidth of each point-to-point link
    --delay=2ms       delay of each point-to-point link
    --rate=1          UDP offered rate in Mbps
    --packetSize=512  UDP packet size in bytes
    --simTime=10      total simulation time in seconds

CHANGE WIRED NODE LAYOUT
------------------------
Open wired_exam.cc and find:
    static std::vector<std::pair<int,int>> edges = {
        {0,1}, {0,2}, {0,3}, {0,4}, {0,5}
    };

Each pair {a,b} means a wired link connects Node a to Node b.
Node numbering starts at 0. For 6 nodes, valid node IDs are 0,1,2,3,4,5.

STAR (default; node 0 is the centre):
    {0,1}, {0,2}, {0,3}, {0,4}, {0,5}

LINE:
    {0,1}, {1,2}, {2,3}, {3,4}, {4,5}

RING:
    {0,1}, {1,2}, {2,3}, {3,4}, {4,5}, {5,0}

TREE:
    {0,1}, {0,2}, {1,3}, {1,4}, {2,5}

TWO BRANCHES:
    {0,1}, {1,2}, {0,3}, {3,4}, {4,5}

MESH-LIKE:
    Start with a connected layout, then add links such as {0,2}, {1,3}, {2,4}.
    Extra links create extra paths. Every link receives a separate 10.1.X.0/24 subnet
    automatically, so do not manually assign subnets.

If the question says 6 nodes, leave --nodes=6 and make sure all edge IDs are from 0 to 5.
If it says 8 nodes, change --nodes=8 AND add edges that connect nodes 6 and 7.

Traffic is from Node 0 to the last node (Node 5 when there are 6 nodes). The routing
tables are populated automatically, so the layout does not need to be a linear chain.

------------------------------------------------------------
2. WIRELESS: wireless_exam.cc
------------------------------------------------------------

Copy:
    cp wireless_exam.cc /path/to/ns-3/scratch/wireless_exam.cc

From the NS-3 root:
    ./ns3 build
    ./ns3 run scratch/wireless_exam

Example with different settings:
    ./ns3 run "scratch/wireless_exam --nodes=6 --rate=2 --packetSize=1024 --simTime=15 --speed=5 --spacing=30"

Arguments:
    --nodes=6        number of Wi-Fi nodes
    --rate=1         UDP offered rate in Mbps
    --packetSize=512 packet size in bytes
    --simTime=10     total simulation time in seconds
    --speed=1        speed of moving nodes in m/s
    --spacing=20     initial radius/spacing in metres

This is a Wi-Fi 802.11g ad hoc network. Wireless nodes share a channel; they are not
connected with physical point-to-point wires. Node 0 sends to the last node.

CHANGE WIRELESS NODE LAYOUT
---------------------------
Find the section that starts with:
    positions->Add(Vector(0.0, 0.0, 0.0)); // node 0
    for (uint32_t i = 1; i < nodeCount; ++i) {
        double angle = (2.0 * M_PI * (i - 1)) / std::max(1u, nodeCount - 1);
        positions->Add(Vector(spacing * std::cos(angle), spacing * std::sin(angle), 0.0));
    }

Current layout: Node 0 is at the centre; other nodes are placed around it.

To make a LINE, replace that position section with:
    for (uint32_t i = 0; i < nodeCount; ++i) {
        positions->Add(Vector(i * spacing, 0.0, 0.0));
    }

To make a GRID, replace it with:
    for (uint32_t i = 0; i < nodeCount; ++i) {
        positions->Add(Vector((i % 3) * spacing, (i / 3) * spacing, 0.0));
    }

To make a STAR-like placement, use node 0 at the centre and put the other nodes around it
(the current default already does this). Remember: wireless layout means physical positions,
not a list of cable links.

Mobility:
    --speed=0  stationary (all moving nodes have zero velocity)
    --speed=1  slow movement
    --speed=5  faster movement
The moving nodes travel in the positive X direction. Increase --spacing to separate nodes
and potentially increase packet loss / reduce received signal power. Results depend on the
simulation and configuration.

------------------------------------------------------------
3. METRICS PRINTED
------------------------------------------------------------

Tx packets / Rx packets:
    Packets sent by the source flow / packets received at destination.

Lost packets:
    Tx packets - Rx packets as reported by FlowMonitor.

Throughput (Mbps):
    received bytes * 8 / (last receive time - first transmit time) / 1,000,000.

Average end-to-end delay (ms):
    FlowMonitor delaySum / received packets * 1000.

Average jitter (ms):
    jitterSum / (received packets - 1) * 1000, when at least two packets are received.

PDR (%):
    received packets / transmitted packets * 100.

Packet loss (%):
    (transmitted packets - received packets) / transmitted packets * 100.

Estimated RTT:
    2 x average one-way delay. This is only a rough estimate; it is not a measured TCP RTT.
    Do not label it as an actual TCP RTT in your exam report.

Received power:
    Wireless program prints a Friis free-space distance-based estimate in dBm. It is not a
    direct PHY measurement and does not model all obstacles/interference.

Retransmissions:
    These templates use UDP, so TCP retransmissions are not applicable. UDP does not
    automatically retransmit lost application packets. If the exam requires retransmissions,
    use a TCP-based scenario or implement application-level UDP retries.

XML output:
    wired-flow-results.xml
    wireless-flow-results.xml

------------------------------------------------------------
4. EXAM QUICK CHECKLIST
------------------------------------------------------------

1. Read the given node count, rate, delay, packet size and simulation time.
2. Wired: update the edges list to match the requested topology.
3. Wireless: update node positions (line/grid/star-like) and speed/spacing.
4. Change command-line values for bandwidth, delay, application rate, packet size, time.
5. Build and run:
       ./ns3 build
       ./ns3 run scratch/wired_exam
       ./ns3 run scratch/wireless_exam
6. Write down throughput, delay, jitter, PDR and packet loss from the actual output.
7. Never invent fixed output numbers: results depend on the supplied question settings.

NOTE
----
The source files are intentionally kept straightforward for lab revision. They use FlowMonitor
for standard flow metrics. They are not a replacement for a TCP congestion-control or TCP
retransmission experiment when the question explicitly requires TCP-specific measurements.
