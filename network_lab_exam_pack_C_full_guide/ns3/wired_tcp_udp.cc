#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

int main(int argc, char *argv[])
{
    std::string rate = "5Mbps";
    std::string delay = "5ms";
    uint32_t packetSize = 1024;
    uint32_t queuePackets = 50;
    double simTime = 30.0;
    double appRate = 2.0; // default offered rate in Mbps

    CommandLine cmd(__FILE__);
    cmd.AddValue("rate", "Point-to-point link data rate", rate);
    cmd.AddValue("delay", "Point-to-point link delay", delay);
    cmd.AddValue("packetSize", "Application packet size in bytes", packetSize);
    cmd.AddValue("queuePackets", "DropTail queue capacity in packets", queuePackets);
    cmd.AddValue("simTime", "Simulation duration in seconds", simTime);
    cmd.AddValue("appRate", "UDP offered rate in Mbps", appRate);
    cmd.Parse(argc, argv);

    NodeContainer nodes;
    nodes.Create(6);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(rate));
    p2p.SetChannelAttribute("Delay", StringValue(delay));
    p2p.SetQueue("ns3::DropTailQueue<Packet>",
                 "MaxSize", StringValue(std::to_string(queuePackets) + "p"));

    // Default topology: Node0 -- Node1 -- Node2 -- Node3 -- Node4 -- Node5.
    // For a branched topology, replace this loop with explicit Install() calls.
    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper address;
    Ipv4InterfaceContainer interfaces;
    for (uint32_t i = 0; i < 5; ++i)
    {
        NetDeviceContainer devices = p2p.Install(nodes.Get(i), nodes.Get(i + 1));
        std::ostringstream subnet;
        subnet << "10.1." << i + 1 << ".0";
        address.SetBase(subnet.str().c_str(), "255.255.255.0");
        interfaces = address.Assign(devices);
        address.NewNetwork();
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t tcpPort = 5000;
    PacketSinkHelper tcpSinkHelper("ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), tcpPort));
    ApplicationContainer tcpSink = tcpSinkHelper.Install(nodes.Get(5));
    tcpSink.Start(Seconds(0.5));
    tcpSink.Stop(Seconds(simTime));

    BulkSendHelper tcpSource("ns3::TcpSocketFactory",
        InetSocketAddress(nodes.Get(5)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), tcpPort));
    // Use the destination's actual address from the final link.
    // The last point-to-point subnet's node-5 address is 10.1.5.2.
    tcpSource.SetAttribute("MaxBytes", UintegerValue(0));
    ApplicationContainer tcpApp = tcpSource.Install(nodes.Get(0));
    tcpApp.Start(Seconds(1.0));
    tcpApp.Stop(Seconds(simTime - 1.0));

    uint16_t udpPort = 5001;
    UdpServerHelper udpServer(udpPort);
    ApplicationContainer serverApps = udpServer.Install(nodes.Get(5));
    serverApps.Start(Seconds(0.5));
    serverApps.Stop(Seconds(simTime));

    UdpClientHelper udpClient(Ipv4Address("10.1.5.2"), udpPort);
    udpClient.SetAttribute("MaxPackets", UintegerValue(1000000));
    udpClient.SetAttribute("Interval", TimeValue(Seconds((packetSize * 8.0) / (appRate * 1000000.0))));
    udpClient.SetAttribute("PacketSize", UintegerValue(packetSize));
    ApplicationContainer clientApps = udpClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(simTime - 1.0));

    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    monitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    auto stats = monitor->GetFlowStats();

    std::cout << "\nSimulation duration: " << simTime << " s\n";
    std::cout << "Configured link rate: " << rate << ", delay: " << delay
              << ", queue: " << queuePackets << " packets, packet size: "
              << packetSize << " bytes\n";
    std::cout << "Flow metrics:\n";
    for (const auto &entry : stats)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(entry.first);
        const FlowMonitor::FlowStats &s = entry.second;
        double duration = s.timeLastRxPacket.GetSeconds() - s.timeFirstTxPacket.GetSeconds();
        double throughput = duration > 0 ? (s.rxBytes * 8.0 / duration / 1e6) : 0.0;
        double avgDelayMs = s.rxPackets > 0 ? (s.delaySum.GetSeconds() * 1000.0 / s.rxPackets) : 0.0;
        double pdr = s.txPackets > 0 ? (100.0 * s.rxPackets / s.txPackets) : 0.0;
        uint64_t lost = s.txPackets > s.rxPackets ? s.txPackets - s.rxPackets : 0;

        std::cout << "Flow " << entry.first << " protocol=" << unsigned(t.protocol)
                  << " " << t.sourceAddress << ":" << t.sourcePort << " -> "
                  << t.destinationAddress << ":" << t.destinationPort << "\n";
        std::cout << "  Tx packets=" << s.txPackets << " Rx packets=" << s.rxPackets
                  << " Lost~=" << lost << " PDR=" << pdr << "%\n";
        std::cout << "  Rx bytes=" << s.rxBytes << " Throughput=" << throughput
                  << " Mbps AvgDelay=" << avgDelayMs << " ms\n";
    }

    Simulator::Destroy();
    return 0;
}
