// Run this file once per variant and compare the printed FlowMonitor metrics.
// Example:
// ./ns3 run "scratch/tcp_variants --variant=NewReno"
// ./ns3 run "scratch/tcp_variants --variant=Cubic"
// ./ns3 run "scratch/tcp_variants --variant=Reno"
// TCP BBR is not present in every NS-3 release. Check your installed release's
// TCP socket TypeId before attempting --variant=Bbr.

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

int main(int argc, char *argv[])
{
    std::string variant = "NewReno";
    std::string rate = "5Mbps";
    std::string delay = "5ms";
    double simTime = 20.0;
    uint32_t packetSize = 1024;

    CommandLine cmd(__FILE__);
    cmd.AddValue("variant", "TCP variant: NewReno, Reno, Cubic (if available)", variant);
    cmd.AddValue("rate", "Link data rate", rate);
    cmd.AddValue("delay", "Link delay", delay);
    cmd.AddValue("simTime", "Simulation duration seconds", simTime);
    cmd.AddValue("packetSize", "TCP application write size", packetSize);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::TcpL4Protocol::SocketType",
                       TypeIdValue(TypeId::LookupByName("ns3::Tcp" + variant)));

    NodeContainer nodes;
    nodes.Create(6);
    InternetStackHelper internet;
    internet.Install(nodes);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(rate));
    p2p.SetChannelAttribute("Delay", StringValue(delay));
    p2p.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue("50p"));

    Ipv4AddressHelper address;
    Ipv4InterfaceContainer lastInterfaces;
    for (uint32_t i = 0; i < 5; ++i)
    {
        auto devices = p2p.Install(nodes.Get(i), nodes.Get(i + 1));
        std::ostringstream subnet;
        subnet << "10.2." << i + 1 << ".0";
        address.SetBase(subnet.str().c_str(), "255.255.255.0");
        lastInterfaces = address.Assign(devices);
        address.NewNetwork();
    }
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t port = 9000;
    PacketSinkHelper sink("ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), port));
    auto sinkApps = sink.Install(nodes.Get(5));
    sinkApps.Start(Seconds(0.5));
    sinkApps.Stop(Seconds(simTime));

    BulkSendHelper source("ns3::TcpSocketFactory",
        InetSocketAddress(lastInterfaces.GetAddress(1), port));
    source.SetAttribute("MaxBytes", UintegerValue(0));
    source.SetAttribute("SendSize", UintegerValue(packetSize));
    auto sourceApps = source.Install(nodes.Get(0));
    sourceApps.Start(Seconds(1.0));
    sourceApps.Stop(Seconds(simTime - 1.0));

    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    monitor->CheckForLostPackets();

    std::cout << "\nTCP variant: " << variant << "\nSimulation duration: "
              << simTime << " s\nLink rate: " << rate << " delay: " << delay << "\n";
    auto classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    for (const auto &e : monitor->GetFlowStats())
    {
        const auto tuple = classifier->FindFlow(e.first);
        const auto &s = e.second;
        double span = s.timeLastRxPacket.GetSeconds() - s.timeFirstTxPacket.GetSeconds();
        double mbps = span > 0 ? s.rxBytes * 8.0 / span / 1e6 : 0;
        double delayMs = s.rxPackets ? s.delaySum.GetSeconds() * 1000.0 / s.rxPackets : 0;
        double pdr = s.txPackets ? 100.0 * s.rxPackets / s.txPackets : 0;
        std::cout << tuple.sourceAddress << " -> " << tuple.destinationAddress
                  << " protocol=" << unsigned(tuple.protocol) << "\n"
                  << "Tx=" << s.txPackets << " Rx=" << s.rxPackets
                  << " PDR=" << pdr << "% Loss~=" << (s.txPackets > s.rxPackets ? s.txPackets-s.rxPackets : 0)
                  << "\nThroughput=" << mbps << " Mbps AvgDelay=" << delayMs << " ms\n";
    }
    Simulator::Destroy();
    return 0;
}
