
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Exp8TCPUDP");

int main(int argc, char *argv[])
{
    // 1. Create six nodes
    NodeContainer nodes;
    nodes.Create(6);

    // 2. Create five point-to-point links in a linear topology
    // Node 0 -- Node 1 -- Node 2 -- Node 3 -- Node 4 -- Node 5

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer d01 = p2p.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer d12 = p2p.Install(nodes.Get(1), nodes.Get(2));
    NetDeviceContainer d23 = p2p.Install(nodes.Get(2), nodes.Get(3));
    NetDeviceContainer d34 = p2p.Install(nodes.Get(3), nodes.Get(4));
    NetDeviceContainer d45 = p2p.Install(nodes.Get(4), nodes.Get(5));

    // 3. Install the Internet stack on all nodes
    InternetStackHelper stack;
    stack.Install(nodes);

    // 4. Assign a different IPv4 subnet to each link
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i01 = address.Assign(d01);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i12 = address.Assign(d12);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer i23 = address.Assign(d23);

    address.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer i34 = address.Assign(d34);

    address.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer i45 = address.Assign(d45);

    // 5. Enable routing between the nodes
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 6. Configure the TCP receiver on Node 5
    uint16_t tcpPort = 5000;

    Address tcpLocalAddress(
        InetSocketAddress(Ipv4Address::GetAny(), tcpPort));

    PacketSinkHelper tcpSinkHelper(
        "ns3::TcpSocketFactory", tcpLocalAddress);

    ApplicationContainer tcpSink =
        tcpSinkHelper.Install(nodes.Get(5));

    tcpSink.Start(Seconds(1.0));
    tcpSink.Stop(Seconds(9.0));

    // 7. Configure the TCP sender on Node 0
    Address tcpRemoteAddress(
        InetSocketAddress(i45.GetAddress(1), tcpPort));

    OnOffHelper tcpSource(
        "ns3::TcpSocketFactory", tcpRemoteAddress);

    tcpSource.SetAttribute("DataRate",
                           StringValue("1Mbps"));
    tcpSource.SetAttribute("PacketSize",
                           UintegerValue(1024));
    tcpSource.SetAttribute("OnTime",
                           StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    tcpSource.SetAttribute("OffTime",
                           StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer tcpApp =
        tcpSource.Install(nodes.Get(0));

    tcpApp.Start(Seconds(2.0));
    tcpApp.Stop(Seconds(8.0));

    // 8. Configure the UDP receiver on Node 4
    uint16_t udpPort = 9000;

    Address udpLocalAddress(
        InetSocketAddress(Ipv4Address::GetAny(), udpPort));

    PacketSinkHelper udpSinkHelper(
        "ns3::UdpSocketFactory", udpLocalAddress);

    ApplicationContainer udpSink =
        udpSinkHelper.Install(nodes.Get(4));

    udpSink.Start(Seconds(1.0));
    udpSink.Stop(Seconds(9.0));

    // 9. Configure the UDP sender on Node 1
    Address udpRemoteAddress(
        InetSocketAddress(i34.GetAddress(1), udpPort));

    OnOffHelper udpSource(
        "ns3::UdpSocketFactory", udpRemoteAddress);

    udpSource.SetAttribute("DataRate",
                           StringValue("1Mbps"));
    udpSource.SetAttribute("PacketSize",
                           UintegerValue(512));
    udpSource.SetAttribute("OnTime",
                           StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    udpSource.SetAttribute("OffTime",
                           StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer udpApp =
        udpSource.Install(nodes.Get(1));

    udpApp.Start(Seconds(2.0));
    udpApp.Stop(Seconds(8.0));

    // 10. Enable FlowMonitor
    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    // 11. Run the simulation
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    // 12. Calculate and display performance metrics
    monitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(
            flowHelper.GetClassifier());

    std::map<FlowId, FlowMonitor::FlowStats> stats =
        monitor->GetFlowStats();

    std::cout << "\n========== EXPERIMENT 8 ==========\n";
    std::cout << "Six-Node Linear TCP/UDP Network\n";
    std::cout << "==================================\n";

    for (const auto &flow : stats)
    {
        Ipv4FlowClassifier::FiveTuple tuple =
            classifier->FindFlow(flow.first);

        double throughput = 0.0;
        double avgDelay = 0.0;
        double pdr = 0.0;
        double packetLoss = 0.0;

        if (flow.second.rxBytes > 0 &&
            flow.second.timeLastRxPacket >
                flow.second.timeFirstTxPacket)
        {
            double duration =
                (flow.second.timeLastRxPacket -
                 flow.second.timeFirstTxPacket).GetSeconds();

            throughput =
                (flow.second.rxBytes * 8.0) /
                duration / 1000000.0;
        }

        if (flow.second.rxPackets > 0)
        {
            avgDelay =
                flow.second.delaySum.GetSeconds() /
                flow.second.rxPackets * 1000.0;
        }

        if (flow.second.txPackets > 0)
        {
            pdr = (flow.second.rxPackets * 100.0) /
                  flow.second.txPackets;

            packetLoss =
                ((flow.second.txPackets -
                  flow.second.rxPackets) * 100.0) /
                flow.second.txPackets;
        }

        std::cout << "\nFlow ID: " << flow.first << "\n";
        std::cout << "Source: " << tuple.sourceAddress
                  << "  Destination: " << tuple.destinationAddress
                  << "\n";
        std::cout << "Protocol: "
                  << (tuple.protocol == 6 ? "TCP" :
                      tuple.protocol == 17 ? "UDP" : "Other")
                  << "\n";

        std::cout << "Packets Sent: "
                  << flow.second.txPackets << "\n";
        std::cout << "Packets Received: "
                  << flow.second.rxPackets << "\n";
        std::cout << "Packets Lost: "
                  << flow.second.lostPackets << "\n";

        std::cout << "Throughput: "
                  << throughput << " Mbps\n";
        std::cout << "Average Delay: "
                  << avgDelay << " ms\n";
        std::cout << "PDR: "
                  << pdr << " %\n";
        std::cout << "Packet Loss: "
                  << packetLoss << " %\n";
    }

    // 13. Save FlowMonitor results
    monitor->SerializeToXmlFile(
        "exp8-flowmon.xml", true, true);

    Simulator::Destroy();

    return 0;
}



//cd ns-3.42(or any other version in system)
// ./ns3 build scratch/filename.cc
// ./ns3 build scratch/filename
