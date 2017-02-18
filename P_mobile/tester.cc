#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/lte-helper-test.h"
#include "ns3/epc-helper-test.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("EpcFirstExample");


static void 
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  //NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd << std::endl;
}

static void
TraceCwnd ()
{
  // Trace changes to the congestion window
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("lte-tcp-single-flow.cwnd");
  Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndChange,stream));
}

int 
main (int argc, char *argv[])
{
  uint32_t maxBytes = 0;
  //uint32_t queueSize = 100;
  DataRate linkRate("1Gbps");
  std::string protocol = "TcpNewReno";    
  double simTime = 5.0;

  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1460));
  Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue (0));
  // Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (524288));
  // Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (524288));
  // Config::SetDefault("ns3::TcpSocket::SlowStartThreshold", UintegerValue (1000000));
  if (protocol == "TcpTahoe")
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe"));
  else
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));

  Ptr<LteHelperTest> lteHelper = CreateObject<LteHelperTest> ();
  Ptr<PointToPointEpcHelperTest>  epcHelper = CreateObject<PointToPointEpcHelperTest> ();
  lteHelper->SetEpcHelper (epcHelper);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  Ptr<Node> pgw1 = epcHelper->GetPgwNode1 ();
  Ptr<Node> pgw2 = epcHelper->GetPgwNode2 ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (2);
  Ptr<Node> remoteHost1 = remoteHostContainer.Get (0);
  Ptr<Node> remoteHost2 = remoteHostContainer.Get (1);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  PointToPointHelper p2ph1;
  p2ph1.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("1Gbps")));
  p2ph1.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph1.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices1 = p2ph1.Install (pgw1, remoteHost1);
  Ipv4AddressHelper ipv4h1;
  ipv4h1.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces1 = ipv4h1.Assign (internetDevices1);

  PointToPointHelper p2ph2;
  p2ph2.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("1Gbps")));
  p2ph2.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph2.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices2 = p2ph2.Install (pgw2, remoteHost2);
  Ipv4AddressHelper ipv4h2;
  ipv4h2.SetBase ("2.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces2 = ipv4h2.Assign (internetDevices2);

  PointToPointHelper p2ph3;
  p2ph2.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("1Gbps")));
  p2ph2.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph2.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices3 = p2ph2.Install (pgw1, pgw2);
  Ipv4AddressHelper ipv4h3;
  ipv4h2.SetBase ("3.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces3 = ipv4h2.Assign (internetDevices3);


  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting1 = ipv4RoutingHelper.GetStaticRouting (remoteHost1->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting2 = ipv4RoutingHelper.GetStaticRouting (remoteHost2->GetObject<Ipv4> ());
  remoteHostStaticRouting1->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  remoteHostStaticRouting2->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);


  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(1);
  ueNodes.Create(1);
  
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));


  Ptr<Node> ueNode = ueNodes.Get (0);
  // Set the default gateway for the UE
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  lteHelper->Attach (ueLteDevs.Get(0), enbLteDevs.Get(0));

  uint16_t port = 8080;  // well-known echo port number
  BulkSendHelper source ("ns3::TcpSocketFactory",InetSocketAddress (ueIpIface.GetAddress (0), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  source.SetAttribute ("SendSize", UintegerValue (1000));
  ApplicationContainer sourceApp1 = source.Install (remoteHostContainer.Get (0));
  sourceApp1.Start (Seconds (1.0));
  sourceApp1.Stop (Seconds (simTime));

 // ApplicationContainer sourceApp2 = source.Install (remoteHostContainer.Get (1));
 // sourceApp2.Start (Seconds (1.0));
 // sourceApp2.Stop (Seconds (simTime));

// Create a PacketSinkApplication and install it on ueNode
  Address sinkAddress (InetSocketAddress (ueIpIface.GetAddress (0), port));
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (ueNodes.Get (0));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (simTime));
 
  // Set up tracing
  //AsciiTraceHelper ascii;
 // p2ph1.EnableAsciiAll (ascii.CreateFileStream ("lte-tcp-single-flow1.tr"));
  p2ph1.EnablePcapAll ("lte-tcp-single-flow1", false);
  //p2ph2.EnableAsciiAll (ascii.CreateFileStream ("lte-tcp-single-flow2.tr"));
  //p2ph2.EnablePcapAll ("lte-tcp-single-flow2", false);
  // Setup tracing for cwnd
  Simulator::Schedule(Seconds(0.00001),&TraceCwnd);
  Simulator::Stop (Seconds (simTime));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

