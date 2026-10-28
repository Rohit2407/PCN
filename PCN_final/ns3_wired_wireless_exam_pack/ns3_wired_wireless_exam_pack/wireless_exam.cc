#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>

using namespace ns3;

int main(int argc, char *argv[])
{
    uint32_t nodeCount = 6;
    std::string standard = "80211g";
    double appRateMbps = 1.0;
    uint32_t packetSize = 512;
    double simTime = 10.0;
    double speed = 1.0;
    double spacing = 20.0;
    uint16_t port = 9000;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nodes", "Number of wireless nodes (default 6)", nodeCount);
    cmd.AddValue("rate", "UDP application rate in Mbps", appRateMbps);
    cmd.AddValue("packetSize", "UDP packet size in bytes", packetSize);
    cmd.AddValue("simTime", "Simulation time in seconds", simTime);
    cmd.AddValue("speed", "Node speed in m/s", speed);
    cmd.AddValue("spacing", "Distance between adjacent positions in metres", spacing);
    cmd.Parse(argc, argv);

    if (nodeCount < 2) {
        std::cout << "Use at least 2 nodes.\n";
        return 1;
    }

    NodeContainer nodes;
    nodes.Create(nodeCount);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue("ErpOfdmRate6Mbps"),
                                 "ControlMode", StringValue("ErpOfdmRate6Mbps"));

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    // Default layout: nodes are placed along the X axis, but wireless nodes
    // communicate directly over the shared channel (not wired links).
    // To make a star-like layout, keep node 0 at the centre and place others
    // around it. See README for the exact lines to change.
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positions = CreateObject<ListPositionAllocator>();
    positions->Add(Vector(0.0, 0.0, 0.0)); // node 0
    for (uint32_t i = 1; i < nodeCount; ++i) {
        double angle = (2.0 * M_PI * (i - 1)) / std::max(1u, nodeCount - 1);
        positions->Add(Vector(spacing * std::cos(angle), spacing * std::sin(angle), 0.0));
    }
    mobility.SetPositionAllocator(positions);
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(nodes);

    for (uint32_t i = 1; i < nodeCount; ++i) {
        nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>()
            ->SetVelocity(Vector(speed, 0.0, 0.0));
    }
    nodes.Get(0)->GetObject<ConstantVelocityMobilityModel>()
        ->SetVelocity(Vector(0.0, 0.0, 0.0));

    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    uint16_t sinkPort = port;
    UdpServerHelper server(sinkPort);
    ApplicationContainer serverApp = server.Install(nodes.Get(nodeCount - 1));
    serverApp.Start(Seconds(0.5));
    serverApp.Stop(Seconds(simTime));

    UdpClientHelper client(interfaces.GetAddress(nodeCount - 1), sinkPort);
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

    std::cout << "\n===== WIRELESS WI-FI RESULTS =====\n";
    std::cout << "Nodes: " << nodeCount << ", standard: 802.11g ad hoc Wi-Fi\n";
    std::cout << "UDP app rate: " << appRateMbps << " Mbps, packet size: " << packetSize
              << " bytes, speed: " << speed << " m/s, spacing: " << spacing
              << " m, simulation time: " << simTime << " s\n";

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

    // Approximate received power using free-space path loss at 2.4 GHz.
    // This is a distance-based estimate, not a measured PHY trace.
    std::cout << "\nApproximate received power estimate (2.4 GHz, Friis free-space):\n";
    for (uint32_t i = 1; i < nodeCount; ++i) {
        Vector a = nodes.Get(0)->GetObject<MobilityModel>()->GetPosition();
        Vector b = nodes.Get(i)->GetObject<MobilityModel>()->GetPosition();
        double d = std::sqrt(std::pow(a.x-b.x,2) + std::pow(a.y-b.y,2) + std::pow(a.z-b.z,2));
        if (d < 1.0) d = 1.0;
        double freq = 2.4e9;
        double c = 299792458.0;
        double wavelength = c / freq;
        double txDbm = 16.0;
        double rxDbm = txDbm + 20.0 * std::log10(wavelength / (4.0 * M_PI * d));
        std::cout << "Node 0 to Node " << i << ": distance=" << d
                  << " m, estimated Rx power=" << rxDbm << " dBm\n";
    }

    std::cout << "TCP retransmissions: not applicable to this UDP example.\n";
    monitor->SerializeToXmlFile("wireless-flow-results.xml", true, true);
    Simulator::Destroy();
    return 0;
}
