#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
using namespace ns3;

struct Result {
  std::string name;
  double throughput, delay, jitter, loss, pdr;
  uint64_t tx, rx, lost;
};

Result RunOne(std::string label, std::string tcpType, std::string bw,
              std::string delay, uint32_t packetSize, double stopTime) {
  // IMPORTANT: Set TCP type BEFORE creating sockets/applications.
  TypeId tid;
  if (!TypeId::LookupByNameFailSafe(tcpType, &tid)) {
    std::cerr << "TCP type unavailable in this NS-3 build: " << tcpType << "\n";
    return {label + " (unavailable)", 0, 0, 0, 0, 0, 0, 0, 0};
  }
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(tid));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(packetSize));

  NodeContainer nodes;
  nodes.Create(6);
  InternetStackHelper internet;
  internet.Install(nodes);

  // EDIT THESE PAIRS for a different six-node topology.
  // Default: two branches, 0-1-2-5 and 0-3-4-5.
  std::vector<std::pair<uint32_t,uint32_t>> edges = {
    {0,1}, {1,2}, {2,5}, {0,3}, {3,4}, {4,5}
  };
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(bw));
  p2p.SetChannelAttribute("Delay", StringValue(delay));
  Ipv4AddressHelper addresses;
  for (uint32_t i=0; i<edges.size(); ++i) {
    NodeContainer pair(nodes.Get(edges[i].first), nodes.Get(edges[i].second));
    NetDeviceContainer dev = p2p.Install(pair);
    std::string subnet = "10.1." + std::to_string(i+1) + ".0";
    addresses.SetBase(Ipv4Address(subnet.c_str()), "255.255.255.0");
    addresses.Assign(dev);
  }
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Find node 5's first IPv4 address (interface 0 is loopback).
  Ipv4Address destination = nodes.Get(5)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
  uint16_t port = 9000;
  PacketSinkHelper sink("ns3::TcpSocketFactory",
                        InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer receiver = sink.Install(nodes.Get(5));
  receiver.Start(Seconds(0.5));
  receiver.Stop(Seconds(stopTime));

  BulkSendHelper sender("ns3::TcpSocketFactory", InetSocketAddress(destination, port));
  sender.SetAttribute("MaxBytes", UintegerValue(0)); // send continuously
  sender.SetAttribute("SendSize", UintegerValue(packetSize));
  ApplicationContainer app = sender.Install(nodes.Get(0));
  app.Start(Seconds(1.0));
  app.Stop(Seconds(stopTime-1.0));

  FlowMonitorHelper fm;
  Ptr<FlowMonitor> monitor = fm.InstallAll();
  Simulator::Stop(Seconds(stopTime));
  Simulator::Run();
  monitor->CheckForLostPackets();
  auto stats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(fm.GetClassifier());
  Result result{label,0,0,0,0,0,0,0,0};
  for (const auto &entry : stats) {
    auto tuple = classifier->FindFlow(entry.first);
    if (tuple.destinationPort != port || tuple.protocol != 6) continue;
    const auto &s = entry.second;
    result.tx += s.txPackets;
    result.rx += s.rxPackets;
    result.lost += s.lostPackets;
    double duration = (s.timeLastRxPacket - s.timeFirstTxPacket).GetSeconds();
    result.throughput = duration > 0 ? s.rxBytes*8.0/duration/1e6 : 0;
    result.delay = s.rxPackets ? s.delaySum.GetSeconds()*1000/s.rxPackets : 0;
    result.jitter = s.rxPackets > 1 ? s.jitterSum.GetSeconds()*1000/(s.rxPackets-1) : 0;
    result.pdr = s.txPackets ? 100.0*s.rxPackets/s.txPackets : 0;
    result.loss = s.txPackets ? 100.0*(s.txPackets-s.rxPackets)/s.txPackets : 0;
  }
  Simulator::Destroy();
  return result;
}

int main(int argc, char *argv[]) {
  std::string bw="5Mbps", delay="2ms";
  uint32_t packetSize=1024;
  double stopTime=12.0;
  CommandLine cmd;
  cmd.AddValue("bw", "Each wired link bandwidth", bw);
  cmd.AddValue("delay", "Each wired link propagation delay", delay);
  cmd.AddValue("packetSize", "TCP segment / send size in bytes", packetSize);
  cmd.AddValue("stopTime", "Simulation duration in seconds (greater than 2)", stopTime);
  cmd.Parse(argc,argv);
  if (stopTime <= 2 || packetSize == 0) { std::cerr << "Invalid arguments\n"; return 1; }

  // In modern NS-3, TcpNewReno is the Reno-family implementation.
  // TcpCubic and TcpBbr are distinct congestion-control algorithms.
  std::vector<Result> results;
  results.push_back(RunOne("NewReno", "ns3::TcpNewReno", bw, delay, packetSize, stopTime));
  results.push_back(RunOne("CUBIC", "ns3::TcpCubic", bw, delay, packetSize, stopTime));
  results.push_back(RunOne("BBR", "ns3::TcpBbr", bw, delay, packetSize, stopTime));

  std::cout << std::fixed << std::setprecision(3);
  std::cout << "\nTCP VARIANT COMPARISON (6 wired nodes)\n";
  std::cout << "Variant       Throughput(Mbps)  Delay(ms)  Jitter(ms)  PDR(%)  Loss(%)  TxPkts  RxPkts\n";
  for (const auto &r : results) {
    std::cout << std::left << std::setw(14) << r.name
              << std::right << std::setw(16) << r.throughput
              << std::setw(11) << r.delay << std::setw(12) << r.jitter
              << std::setw(9) << r.pdr << std::setw(9) << r.loss
              << std::setw(8) << r.tx << std::setw(8) << r.rx << "\n";
  }
  std::cout << "NOTE: FlowMonitor TCP tx/rx counts include retransmitted IP packets;\n"
            << "tx-rx is NOT a reliable TCP retransmission count.\n"
            << "RTT and congestion window are not measured by this simple template.\n";
}
