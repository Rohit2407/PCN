#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <sstream>

using namespace ns3;

// Change this list to change the layout. Node numbers start at 0.
// LINE:  {{0,1},{1,2},{2,3},{3,4},{4,5}}
// STAR:  {{0,1},{0,2},{0,3},{0,4},{0,5}}
// RING:  {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0}}
// TREE:  {{0,1},{0,2},{1,3},{1,4},{2,5}}
// MESH:  connect more pairs, e.g. add {0,2}, {1,3}, etc.
static std::vector<std::pair<int,int>> edges = {
    {0,1}, {0,2}, {0,3}, {0,4}, {0,5} // default: STAR
};

int main(int argc, char *argv[])
{
    uint32_t nodeCount = 6;
    std::string dataRate = "5Mbps";
    std::string delay = "2ms";
    double appRateMbps = 1.0;
    uint32_t packetSize = 512;
    double simTime = 10.0;
    uint16_t port = 9000;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nodes", "Number of nodes (default 6)", nodeCount);
    cmd.AddValue("bw", "Bandwidth on each wired link", dataRate);
    cmd.AddValue("delay", "Propagation delay on each wired link", delay);
    cmd.AddValue("rate", "UDP application rate in Mbps", appRateMbps);
    cmd.AddValue("packetSize", "UDP packet size in bytes", packetSize);
    cmd.AddValue("simTime", "Simulation time in seconds", simTime);
    cmd.Parse(argc, argv);

    if (nodeCount < 2) {
        std::cout << "Use at least 2 nodes.\n";
        return 1;
    }
    for (const auto &e : edges) {
        if (e.first >= (int)nodeCount || e.second >= (int)nodeCount) {
            std::cout << "Topology edge refers to a node outside --nodes. Update edges or --nodes.\n";
            return 1;
        }
    }

    NodeContainer nodes;
    nodes.Create(nodeCount);

    InternetStackHelper internet;
    internet.Install(nodes);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(dataRate));
    p2p.SetChannelAttribute("Delay", StringValue(delay));

    Ipv4AddressHelper address;
    std::vector<NetDeviceContainer> devices;
    for (uint32_t i = 0; i < edges.size(); ++i) {
        NetDeviceContainer link = p2p.Install(nodes.Get(edges[i].first), nodes.Get(edges[i].second));
        std::ostringstream subnet;
        subnet << "10.1." << (i + 1) << ".0";
        address.SetBase(Ipv4Address(subnet.str().c_str()), "255.255.255.0");
        address.Assign(link);
        devices.push_back(link);
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t sinkPort = port;
    UdpServerHelper server(sinkPort);
    ApplicationContainer serverApp = server.Install(nodes.Get(nodeCount - 1));
    serverApp.Start(Seconds(0.5));
    serverApp.Stop(Seconds(simTime));

    UdpClientHelper client(nodes.Get(nodeCount - 1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), sinkPort);
    client.SetAttribute("MaxPackets", UintegerValue(1000000));
    client.SetAttribute("Interval", TimeValue(Seconds((packetSize * 8.0) / (appRateMbps * 1000000.0))));
    client.SetAttribute("PacketSize", UintegerValue(packetSize));
    ApplicationContainer clientApp = client.Install(nodes.Get(0));
    clientApp.Start(Seconds(1.0));
    clientApp.Stop(Seconds(simTime - 0.1));

    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    monitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    std::cout << "\n===== WIRED NETWORK RESULTS =====\n";
    std::cout << "Nodes: " << nodeCount << ", links: " << edges.size()
              << ", link bandwidth: " << dataRate << ", link delay: " << delay << "\n";
    std::cout << "UDP app rate: " << appRateMbps << " Mbps, packet size: " << packetSize
              << " bytes, simulation time: " << simTime << " s\n";

    for (const auto &item : stats) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(item.first);
        const FlowMonitor::FlowStats &s = item.second;
        if (t.destinationPort != sinkPort) continue;

        double duration = (s.timeLastRxPacket - s.timeFirstTxPacket).GetSeconds();
        double throughput = duration > 0 ? (s.rxBytes * 8.0 / duration / 1000000.0) : 0.0;
        double avgDelay = s.rxPackets > 0 ? (s.delaySum.GetSeconds() * 1000.0 / s.rxPackets) : 0.0;
        double pdr = s.txPackets > 0 ? (100.0 * s.rxPackets / s.txPackets) : 0.0;
        double loss = s.txPackets > 0 ? (100.0 * (s.txPackets - s.rxPackets) / s.txPackets) : 0.0;
        double avgJitter = s.rxPackets > 1 ? (s.jitterSum.GetSeconds() * 1000.0 / (s.rxPackets - 1)) : 0.0;

        std::cout << "\nFlow " << item.first << " (" << t.sourceAddress << " -> "
                  << t.destinationAddress << ", UDP port " << t.destinationPort << ")\n";
        std::cout << "Tx packets: " << s.txPackets << "\n";
        std::cout << "Rx packets: " << s.rxPackets << "\n";
        std::cout << "Lost packets: " << (s.txPackets - s.rxPackets) << "\n";
        std::cout << "Tx bytes: " << s.txBytes << ", Rx bytes: " << s.rxBytes << "\n";
        std::cout << "Throughput: " << throughput << " Mbps\n";
        std::cout << "Average end-to-end delay: " << avgDelay << " ms\n";
        std::cout << "Average jitter: " << avgJitter << " ms\n";
        std::cout << "PDR: " << pdr << " %\n";
        std::cout << "Packet loss: " << loss << " %\n";
        std::cout << "Estimated RTT (2 x one-way delay; not a measured TCP RTT): "
                  << 2.0 * avgDelay << " ms\n";
        std::cout << "TCP retransmissions: N/A (this file sends UDP)\n";
    }

    monitor->SerializeToXmlFile("wired-flow-results.xml", true, true);
    Simulator::Destroy();
    return 0;
}
