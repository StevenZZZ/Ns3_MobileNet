/* Here you have a flow which starts at 5s and then there is gateway switch at 10s, I have tracked the congestion window. The throughput is also calculated but it can be found at different file-lte2
Basically, you have two gateways connected to a single ENB and then there is path switch between the two gateway*/
#include <fstream>
#include <string>
#include <cassert>
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/packet-sink.h"
//#include "ns3/gtk-config-store.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("TCPSingleFlow");

static void
CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
}

int
main (int argc, char *argv[])
{
   Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno")); //set the TCP protocol to newreno
  // Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));
 // Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
  Config::SetDefault ("ns3::PointToPointEpcHelper::S1uLinkDelay", TimeValue (MilliSeconds (2))); // s1 link delay
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> (); 
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> (); //calling EPC helper
  lteHelper->SetEpcHelper (epcHelper);
  //lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  Ptr<Node> pgw1 = epcHelper->GetPgwNode (); //P-GW, I have used one PGW from lte helper and other one as normal node
  //Ptr<Node> pgw2 = epcHelper->GetPgwNode2 ();

  NodeContainer c;
  c.Create (4);

  InternetStackHelper internet;
  internet.Install (c); //installing protocol stack 

  NodeContainer ueNodes; //ue and enb creation
  NodeContainer enbNodes;
  enbNodes.Create(1);
  ueNodes.Create(1);
  
  
  Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator> (); //assigning position to ue and enb
  Ptr<ListPositionAllocator> positionAlloc2 = CreateObject<ListPositionAllocator> ();
  
  
  MobilityHelper mobility; //mobility models
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  positionAlloc1->Add (Vector (0, 50, 0));
  mobility.SetPositionAllocator(positionAlloc1);
  mobility.Install(enbNodes);
  //mobility.Install(ueNodes);

  MobilityHelper ueMobility;
  ueMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  positionAlloc2->Add (Vector (0, 100, 0));
  ueMobility.SetPositionAllocator(positionAlloc2);
  ueMobility.Install (ueNodes);


  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));


  NodeContainer n0n1 = NodeContainer (c.Get (0), c.Get (1)); // creation of links
  NodeContainer n1n2 = NodeContainer (c.Get (1), c.Get (2));
  NodeContainer n2S2 = NodeContainer (c.Get (2), c.Get (3));
  NodeContainer EbS2 = NodeContainer (enbNodes.Get(0),c.Get (3));
  NodeContainer S1n1 = NodeContainer (pgw1,c.Get(1));
  
  // Create the Internet
  PointToPointHelper p2ph1;
  p2ph1.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("10Gbps")));
  p2ph1.SetDeviceAttribute ("Mtu", UintegerValue (2000));
  p2ph1.SetChannelAttribute ("Delay", TimeValue (Seconds (0.002)));
  
  PointToPointHelper p2ph2;
  p2ph2.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("10Gbps")));
  p2ph2.SetDeviceAttribute ("Mtu", UintegerValue (2000));
  p2ph2.SetChannelAttribute ("Delay", TimeValue (Seconds (0.004)));

  NetDeviceContainer internetDevices1 = p2ph1.Install (n0n1);
  NetDeviceContainer internetDevices2 = p2ph2.Install (n1n2);
  NetDeviceContainer internetDevices3 = p2ph1.Install (n2S2);
  NetDeviceContainer internetDevices4 = p2ph1.Install (EbS2);
  NetDeviceContainer internetDevices5 = p2ph1.Install (S1n1);

  Ipv4AddressHelper ipv4h1;
  Ipv4AddressHelper ipv4h2;
  Ipv4AddressHelper ipv4h3;
  Ipv4AddressHelper ipv4h4;
  Ipv4AddressHelper ipv4h5;
  ipv4h1.SetBase ("1.0.0.0", "255.0.0.0");
  ipv4h2.SetBase ("2.0.0.0", "255.0.0.0");
  ipv4h3.SetBase ("3.0.0.0", "255.0.0.0");
  ipv4h4.SetBase ("4.0.0.0", "255.0.0.0");
  ipv4h5.SetBase ("5.0.0.0", "255.0.0.0");
  
  ipv4h1.Assign (internetDevices1);
  ipv4h2.Assign (internetDevices2);
  ipv4h3.Assign (internetDevices3);
  ipv4h4.Assign (internetDevices4);
  ipv4h5.Assign (internetDevices5);



  Ptr<Node> n0 = c.Get (0);
  Ptr<Node> n1 = c.Get (1);
  Ptr<Node> n2 = c.Get (2);
  Ptr<Node> enb = enbNodes.Get(0);
  Ptr<Node> pgw2 = c.Get (3);

  Ipv4StaticRoutingHelper ipv4RoutingHelper; // installing ip lookup rules
  Ptr<Ipv4StaticRouting> Routing1 = ipv4RoutingHelper.GetStaticRouting (n0->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing2 = ipv4RoutingHelper.GetStaticRouting (n1->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing3 = ipv4RoutingHelper.GetStaticRouting (pgw2->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing4 = ipv4RoutingHelper.GetStaticRouting (enb->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing5 = ipv4RoutingHelper.GetStaticRouting (n2->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing6 = ipv4RoutingHelper.GetStaticRouting (pgw1->GetObject<Ipv4> ());

  Routing1->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  Routing2->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 2);
  Routing2->SetDefaultRoute (Ipv4Address ("7.0.0.0"), 3);
  Routing3->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 2);
  Routing3->SetDefaultRoute (Ipv4Address ("1.0.0.1"), 1);
  Routing4->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  Routing5->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 2);
  Routing6->SetDefaultRoute (Ipv4Address ("1.0.0.1"), 3);
  
  // Install Mobility Model


  // Install LTE Devices to the nodes

  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }


  // Attach one UE per eNodeB
  for (uint16_t i = 0; i < 1; i++)
      {
        lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i));
        // side effect: the default EPS bearer will be activated
      }

  // Install and start applications on UEs and remote host

    uint16_t port = 9;  // well-known echo port number

    BulkSendHelper source ("ns3::TcpSocketFactory", //installing application 
                         InetSocketAddress (ueIpIface.GetAddress (0), port));
     // Set the amount of data to send in bytes.  Zero is unlimited.
      source.SetAttribute ("MaxBytes", UintegerValue (0));
    // Set the segment size
     source.SetAttribute ("SendSize", UintegerValue (10000));
  ApplicationContainer sourceApps = source.Install (c.Get (0));
  sourceApps.Start (Seconds (5.0));
  sourceApps.Stop (Seconds (15.0));

  // Create a PacketSinkApplication and install it on node 1
  PacketSinkHelper sink ("ns3::TcpSocketFactory", //installing sink
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (ueNodes.Get (0));
  sinkApps.Start (Seconds (1.0));
  sinkApps.Stop (Seconds (15.0));
  //Simulator::Schedule(Seconds(0.001),&TraceCwnd);

  Ptr<Socket> ns3TcpSocket1 = Socket::CreateSocket (c.Get (0), TcpSocketFactory::GetTypeId ());
  ns3TcpSocket1->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));// keeping track of congestion window
  //Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndChange));

  //ueLteDevs.Get(0)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));


  //lteHelper->EnableTraces ();
  // Uncomment to enable PCAP tracing
  p2ph1.EnablePcapAll("multi");

    Ptr<Ipv4> ipv41 = n1->GetObject<Ipv4> ();
    Simulator::Schedule (Seconds(10), &Ipv4::SetDown, ipv41, 2);
    //Simulator::Schedule (Seconds(10.001), &Ipv4::SetDown, ipv41, 3);
    //Simulator::Schedule (Seconds(10.002), &Ipv4::SetUp, ipv41, 3);

    
    Ipv4GlobalRoutingHelper g;
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dynamic-global-routing.routes", std::ios::out);
    g.PrintRoutingTableAllAt (Seconds (8), routingStream);
  

   FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

  
  Simulator::Stop(Seconds(15));
  
  Simulator::Run();

  
  /*GtkConfigStore config;
  config.ConfigureAttributes();*/

  Simulator::Destroy();

}

