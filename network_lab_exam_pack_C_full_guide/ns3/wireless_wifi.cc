#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

int main(int argc, char *argv[])
{
    uint32_t nodeCount = 6;
    double simTime = 30.0;
    double distance = 20.0;
    uint32_t packetSize = 1024;
    std::string appRate = "2Mbps";

    CommandLine cmd(__FILE__);
    cmd.AddValue("nodes", "Number of Wi-Fi nodes", nodeCount);
    cmd.AddValue("simTime", "Simulation duration seconds", simTime);
    cmd.AddValue("distance", "Distance between nodes in metres", distance);
    cmd.AddValue("packetSize", "UDP packet size bytes", packetSize);
    cmd.AddValue("appRate", "UDP offered data rate", appRate);
    cmd.Parse(argc, argv);

    NodeContainer nodes;
    nodes.Create(nodeCount);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
        "DataMode", StringValue("ErpOfdmRate24Mbps"),
        "ControlMode", StringValue("ErpOfdmRate6Mbps"));

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positions = CreateObject<ListPositionAllocator>();
    for (uint32_t i = 0; i < nodeCount; ++i)
        positions->Add(Vector(i * distance, 0.0, 0.0));
    mobility.SetPositionAllocator(positions);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    InternetStackHelper internet;
    internet.Install(nodes);
    Ipv4AddressHelper address;
    address.SetBase("10.3.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    uint16_t port = 7000;
    UdpServerHelper server(port);
    auto serverApps = server.Install(nodes.Get(nodeCount - 1));
    serverApps.Start(Seconds(0.5));
    serverApps.Stop(Seconds(simTime));

    UdpClientHelper client(interfaces.GetAddress(nodeCount - 1), port);
    client.SetAttribute("MaxPackets", UintegerValue(1000000));
    client.SetAttribute("Interval", TimeValue(Seconds(1024 * 8.0 / 2e6)));
    client.SetAttribute("PacketSize", UintegerValue(packetSize));
    client.SetAttribute("MaxPackets", UintegerValue(1000000));
    // Override the default interval to match the chosen application rate.
    double rateMbps = 2.0;
    if (appRate.find("Mbps") != std::string::npos)
        rateMbps = std::stod(appRate.substr(0, appRate.find("Mbps")));
    else if (appRate.find("Kbps") != std::string::npos)
        rateMbps = std::stod(appRate.substr(0, appRate.find("Kbps"))) / 1000.0;
    client.SetAttribute("Interval", TimeValue(Seconds(packetSize * 8.0 / (rateMbps * 1e6))));
    auto clientApps = client.Install(nodes.Get(0));
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(simTime - 1.0));

    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    monitor->CheckForLostPackets();

    std::cout << "\nWireless Wi-Fi ad-hoc experiment\nNodes=" << nodeCount
              << " Distance step=" << distance << " m SimTime=" << simTime
              << " s AppRate=" << appRate << "\n";
    auto classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    for (const auto &e : monitor->GetFlowStats())
    {
        auto tuple = classifier->FindFlow(e.first);
        const auto &s = e.second;
        double span = s.timeLastRxPacket.GetSeconds() - s.timeFirstTxPacket.GetSeconds();
        double mbps = span > 0 ? s.rxBytes * 8.0 / span / 1e6 : 0;
        double delayMs = s.rxPackets ? s.delaySum.GetSeconds() * 1000.0 / s.rxPackets : 0;
        double pdr = s.txPackets ? 100.0 * s.rxPackets / s.txPackets : 0;
        std::cout << tuple.sourceAddress << " -> " << tuple.destinationAddress
                  << " Tx=" << s.txPackets << " Rx=" << s.rxPackets
                  << " Loss~=" << (s.txPackets > s.rxPackets ? s.txPackets-s.rxPackets : 0)
                  << " PDR=" << pdr << "% Throughput=" << mbps
                  << " Mbps AvgDelay=" << delayMs << " ms\n";
    }
    Simulator::Destroy();
    return 0;
}
