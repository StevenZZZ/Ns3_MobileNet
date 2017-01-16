/*this file gives you throughput of the multigateway scenario.Same as LTE1 file , flow starts at 1s and then there is gateway switch at 5s.This same file can also be used for tracking UDP throughput.
similar to the previous one 2pgw conected to one single enb*/
#include <iostream>
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
//#include "ns3/gtk-config-store.h"

using namespace ns3;

Ptr<PacketSink> sinks;                         /* Pointer to the packet sink application */
//uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */

uint32_t pktcounter = 0;
uint32_t oldcounter = 0;

static void
RxDrop (Ptr<const Packet> p)
{
  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
}

void ReceivePacket (Ptr<const Packet> packet, const Address &)
{
//NS_LOG_DEBUG ("Received one packet!");
 pktcounter ++;
}

void Throughput()//calculation
{
  long double  throughput = ((pktcounter - oldcounter)*1000*8/0.1/1024/1024);
  std::cout <<"\n" << throughput;
  
  oldcounter = pktcounter; 
  Simulator::Schedule(Seconds (0.1), &Throughput);
}

NS_LOG_COMPONENT_DEFINE ("EpcFirstExample");

int
main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::LteEnbRrc::DefaultTransmissionMode",UintegerValue (2));
  Config::SetDefault ("ns3::PointToPointEpcHelper::S1uLinkDelay", TimeValue (MilliSeconds (2)));
  //Config::SetDefault ("ns3::RadioEnvironmentMapHelper::Bandwidth",UintegerValue (100));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));
  uint8_t bandwidth = 100;
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth));
 
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  Ptr<Node> pgw1 = epcHelper->GetPgwNode ();
 
  NodeContainer c;
  c.Create (4);

  InternetStackHelper internet;
  internet.Install (c);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(1);
  ueNodes.Create(1);

  Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> positionAlloc2 = CreateObject<ListPositionAllocator> ();
  
  
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  positionAlloc1->Add (Vector (0, 50, 0));
  mobility.SetPositionAllocator(positionAlloc1);
  mobility.Install(enbNodes);
  //mobility.Install(ueNodes);

  MobilityHelper ueMobility;
  ueMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  positionAlloc2->Add (Vector (0, 100, 0));
  ueMobility.SetPositionAllocator(positionAlloc2);
  ueMobility.Install (ueNodes);

  ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (10, 0, 0));

  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));


  NodeContainer n0n1 = NodeContainer (c.Get (0), c.Get (1));
  NodeContainer n1n2 = NodeContainer (c.Get (1), c.Get (2));
  NodeContainer n2S2 = NodeContainer (c.Get (2), c.Get (3));
  NodeContainer EbS2 = NodeContainer (enbNodes.Get(0),c.Get(3));
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
  Ptr<Node> pgw2 = c.Get(3);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;//iprules
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

    uint16_t port = 9;   // Discard port (RFC 863)
    OnOffHelper onoff ("ns3::TcpSocketFactory",InetSocketAddress (ueIpIface.GetAddress (0), port));
    onoff.SetConstantRate (DataRate ("5Mbps"));
    onoff.SetAttribute ("PacketSize", UintegerValue (1000));
    onoff.SetAttribute ("MaxBytes", UintegerValue (0));

    ApplicationContainer apps = onoff.Install (c.Get(0));
    apps.Start (Seconds (1.0));
    apps.Stop (Seconds (10.0));

    // n0 -> n2
    PacketSinkHelper sink ("ns3::TcpSocketFactory",Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
    apps = sink.Install (ueNodes.Get(0));
    sinks = StaticCast<PacketSink> (apps.Get (1));
    apps.Start (Seconds (0.5));
    apps.Stop (Seconds (10.0));

  Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",MakeCallback (&ReceivePacket));
  
  Simulator::Schedule(Seconds (0.1), &Throughput);//calling the throughput function

 
  p2ph1.EnablePcapAll("multi");//pcap files

    Ptr<Ipv4> ipv41 = n1->GetObject<Ipv4> ();
    Simulator::Schedule (Seconds(5), &Ipv4::SetDown, ipv41, 2);

    Ipv4GlobalRoutingHelper g;//printing of routing table
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dynamic-global-routing.routes", std::ios::out);
    g.PrintRoutingTableAllAt (Seconds (8), routingStream);
  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

   FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();
  Simulator::Stop(Seconds(15));

  enbLteDevs.Get(0)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));
  
  Simulator::Run();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ()); //flowmonitor application to keep of the sent and received packet.
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();//collecting the statistics 
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (( t.destinationAddress == "7.0.0.2"))
      {
          std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      	  std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstRxPacket.GetSeconds())/1024/1024  << " Mbps\n";
          std::cout << "  Time To Travel: " << (i->second.timeFirstRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) << "\n"; 
      }
     }
    monitor->SerializeToXmlFile("prasad.xml", true, true); 
  

  Simulator::Destroy();
  return 0;

}

