/* This file gives you about the performance of TCP during multi-path switching.We have 3 multiple path of different RTT and there is path switch among different path at different time instant*/
#include <iostream>
#include <string>
#include <fstream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("FifthScriptExample");

Ptr<PacketSink> sinks;

//static void
//CwndTracer (Ptr<OutputStreamWrapper>stream, uint32_t oldval, uint32_t newval)
//{
//  double new_time = Simulator::Now().GetSeconds();
//  *stream->GetStream() << new_time << " " << newval << std::endl;
//}

//static void
//TraceCwnd ()
//{
//  Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper> (&std::cout);
//  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndTracer, stream));
//}

uint32_t pktcounter = 0;
uint32_t oldcounter = 0;

void ReceivePacket (Ptr<const Packet> packet, const Address &)
{
NS_LOG_DEBUG ("Received one packet!");
 pktcounter ++;
}

void Throughput() //calculation
{
  long double  throughput = ((pktcounter - oldcounter)*512*8/0.01/1024/1024);
  std::cout <<"\n" << throughput;
  
  oldcounter = pktcounter; 
  Simulator::Schedule(Seconds (0.01), &Throughput);
}
//uint32_t pktcounter = 0;


//void ReceivePacket (Ptr<const Packet> packet, const Address &)
//{
//NS_LOG_DEBUG ("Received one packet!");
//pktcounter ++;
//NS_LOG_UNCOND (pktcounter <<" " << Simulator::Now ().GetSeconds ());
//}

//void TxTrace(Ptr<const Packet> p)
//{
// pktcounter ++;
// NS_LOG_UNCOND (pktcounter <<" " << Simulator::Now ().GetSeconds ());
//}

//static void
//RxDrop (Ptr<const Packet> p)
//{
//  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
//}

int 
main (int argc, char *argv[])
{
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (4000000));

   NodeContainer c;
   c.Create (8);
   
   DataRate linkRate1("50Mbps");//all links have same linkrate
   //DataRate linkRate2("15Mbps");

  // Create the point-to-point link required by the topology
  PointToPointHelper p2p1;
  p2p1.SetDeviceAttribute ("DataRate", DataRateValue(linkRate1));
  p2p1.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2p1.SetChannelAttribute ("Delay", StringValue ("3ms"));

  PointToPointHelper p2p2;
  p2p2.SetDeviceAttribute ("DataRate", DataRateValue(linkRate1));
  p2p2.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2p2.SetChannelAttribute ("Delay", StringValue ("1ms"));

  PointToPointHelper p2p3;
  p2p3.SetDeviceAttribute ("DataRate", DataRateValue(linkRate1));
  p2p3.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2p3.SetChannelAttribute ("Delay", StringValue ("0.5ms"));

  NodeContainer n0n1 = NodeContainer (c.Get (0), c.Get (1));//topology creation
  NodeContainer n1n2 = NodeContainer (c.Get (1), c.Get (2));
  NodeContainer n1n3 = NodeContainer (c.Get (1), c.Get (3));
  NodeContainer n3n4 = NodeContainer (c.Get (3), c.Get (4));
  NodeContainer n4n6 = NodeContainer (c.Get (4), c.Get (6));
  NodeContainer n2n6 = NodeContainer (c.Get (2), c.Get (6));
  NodeContainer n1n5 = NodeContainer (c.Get (1), c.Get (5));
  NodeContainer n5n6 = NodeContainer (c.Get (5), c.Get (6));
  NodeContainer n6n7 = NodeContainer (c.Get (6), c.Get (7));
 

  NetDeviceContainer device1 = p2p1.Install (n0n1);
  NetDeviceContainer device2 = p2p1.Install (n1n2);
  NetDeviceContainer device3 = p2p2.Install (n1n3);
  NetDeviceContainer device4 = p2p2.Install (n3n4);
  NetDeviceContainer device5 = p2p2.Install (n4n6);
  NetDeviceContainer device6 = p2p3.Install (n1n5);
  NetDeviceContainer device7 = p2p1.Install (n2n6);
  NetDeviceContainer device8 = p2p3.Install (n5n6);
  NetDeviceContainer device9 = p2p1.Install (n6n7);


  //Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  //em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  //device5.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

  // Install the internet stack on the nodes
  InternetStackHelper internet;
  internet.Install (c);

  // We've got the "hardware" in place.  Now we need to add IP addresses.
  Ipv4AddressHelper ipv4h1;
  Ipv4AddressHelper ipv4h2;
  Ipv4AddressHelper ipv4h3;
  Ipv4AddressHelper ipv4h4;
  Ipv4AddressHelper ipv4h5;
  Ipv4AddressHelper ipv4h6;
  Ipv4AddressHelper ipv4h7;
  Ipv4AddressHelper ipv4h8;
  Ipv4AddressHelper ipv4h9;

  ipv4h1.SetBase ("1.0.0.0", "255.0.0.0");
  ipv4h2.SetBase ("2.0.0.0", "255.0.0.0");
  ipv4h3.SetBase ("3.0.0.0", "255.0.0.0");
  ipv4h4.SetBase ("4.0.0.0", "255.0.0.0");
  ipv4h5.SetBase ("5.0.0.0", "255.0.0.0");
  ipv4h6.SetBase ("6.0.0.0", "255.0.0.0");
  ipv4h7.SetBase ("7.0.0.0", "255.0.0.0");
  ipv4h8.SetBase ("8.0.0.0", "255.0.0.0");
  ipv4h9.SetBase ("9.0.0.0", "255.0.0.0");
  
   Ipv4InterfaceContainer ip1 = ipv4h1.Assign (device1);
   Ipv4InterfaceContainer ip2 = ipv4h2.Assign (device2);
   Ipv4InterfaceContainer ip3 = ipv4h3.Assign (device3);
   Ipv4InterfaceContainer ip4 = ipv4h4.Assign (device4);
   Ipv4InterfaceContainer ip5 = ipv4h5.Assign (device5);
   Ipv4InterfaceContainer ip6 = ipv4h6.Assign (device6);
   Ipv4InterfaceContainer ip7 = ipv4h7.Assign (device7);
   Ipv4InterfaceContainer ip8 = ipv4h8.Assign (device8);
   Ipv4InterfaceContainer ip9 = ipv4h9.Assign (device9);

  Ptr<Node> n0 = c.Get (0);
  Ptr<Node> n1 = c.Get (1);
  Ptr<Node> n2 = c.Get (2);
  Ptr<Node> n3 = c.Get (3);
  Ptr<Node> n4 = c.Get (4);
  Ptr<Node> n5 = c.Get (5);
  Ptr<Node> n6 = c.Get (6);
  Ptr<Node> n7 = c.Get (7);
   
  Ipv4StaticRoutingHelper ipv4RoutingHelper; //ip lookup address
  Ptr<Ipv4StaticRouting> Routing1 = ipv4RoutingHelper.GetStaticRouting (n0->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing2 = ipv4RoutingHelper.GetStaticRouting (n1->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing3 = ipv4RoutingHelper.GetStaticRouting (n2->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing4 = ipv4RoutingHelper.GetStaticRouting (n3->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing5 = ipv4RoutingHelper.GetStaticRouting (n4->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing6 = ipv4RoutingHelper.GetStaticRouting (n5->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing7 = ipv4RoutingHelper.GetStaticRouting (n6->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing8 = ipv4RoutingHelper.GetStaticRouting (n7->GetObject<Ipv4> ());
  

  Routing1->AddNetworkRouteTo (Ipv4Address ("9.0.0.2"), Ipv4Mask ("255.0.0.0"), 1);

  Routing2->AddNetworkRouteTo (Ipv4Address ("9.0.0.2"), Ipv4Mask ("255.0.0.0"), 3);
  Routing2->AddNetworkRouteTo (Ipv4Address ("9.0.0.2"), Ipv4Mask ("255.0.0.0"), 2);
  Routing2->AddNetworkRouteTo (Ipv4Address ("1.0.0.1"), Ipv4Mask ("255.0.0.0"), 1);
  Routing2->SetDefaultRoute (Ipv4Address ("9.0.0.2"), 4);

  Routing3->AddNetworkRouteTo (Ipv4Address ("9.0.0.2"), Ipv4Mask ("255.0.0.0"), 2);
  Routing3->AddNetworkRouteTo (Ipv4Address ("1.0.0.1"), Ipv4Mask ("255.0.0.0"), 1);

  Routing4->AddNetworkRouteTo (Ipv4Address ("9.0.0.2"), Ipv4Mask ("255.0.0.0"), 2);
  Routing4->AddNetworkRouteTo (Ipv4Address ("1.0.0.1"), Ipv4Mask ("255.0.0.0"), 1);

  Routing5->AddNetworkRouteTo (Ipv4Address ("9.0.0.2"), Ipv4Mask ("255.0.0.0"), 2);
  Routing5->AddNetworkRouteTo (Ipv4Address ("1.0.0.1"), Ipv4Mask ("255.0.0.0"), 1);

  Routing6->AddNetworkRouteTo (Ipv4Address ("9.0.0.2"), Ipv4Mask ("255.0.0.0"), 2);
  Routing6->AddNetworkRouteTo (Ipv4Address ("1.0.0.1"), Ipv4Mask ("255.0.0.0"), 1);

  Routing7->AddNetworkRouteTo (Ipv4Address ("9.0.0.2"), Ipv4Mask ("255.0.0.0"), 4);
  Routing7->SetDefaultRoute (Ipv4Address ("1.0.0.1"), 3);

  Routing8->AddNetworkRouteTo (Ipv4Address ("1.0.0.1"), Ipv4Mask ("255.0.0.0"), 1);
  
   uint16_t port = 9;   // Discard port (RFC 863) //application
    BulkSendHelper onoff ("ns3::TcpSocketFactory",InetSocketAddress (ip9.GetAddress (1), port));
    //onoff.SetConstantRate (DataRate ("0.1Mbps"));
    //onoff.SetAttribute ("PacketSize", UintegerValue (1500));
    onoff.SetAttribute ("MaxBytes", UintegerValue (0));

    ApplicationContainer apps = onoff.Install (c.Get(0));
    apps.Start (Seconds (2.0));
    apps.Stop (Seconds (10.0));

    // n0 -> n2
    PacketSinkHelper sink ("ns3::TcpSocketFactory",Address (InetSocketAddress (Ipv4Address::GetAny (), port)));//sink application
    apps = sink.Install (c.Get(7));
    sinks = StaticCast<PacketSink> (apps.Get (1));
    apps.Start (Seconds (1.0));
    //apps.Stop (Seconds (10.0));
   
   Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",MakeCallback (&ReceivePacket));
  Simulator::Schedule(Seconds (0.01), &Throughput);
  //Simulator::Schedule(Seconds(2.0001), &TraceCwnd);
 //Config::ConnectWithoutContext("/NodeList/2/ApplicationList/*/$ns3::PacketSink/Rx",MakeCallback (&ReceivePacket));
 //  Ptr<BulkSendApplication> app = DynamicCast<BulkSendApplication> (c.Get(0)->GetApplication(0));
 // app->TraceConnectWithoutContext("Tx", MakeCallback(&TxTrace));
  
    Ptr<Ipv4> ipv41 = n1->GetObject<Ipv4> ();//path switch
   //// Ptr<Ipv4> ipv42 = n3->GetObject<Ipv4> ();
    Simulator::Schedule (Seconds(4), &Ipv4::SetDown, ipv41, 2);
    Simulator::Schedule (Seconds(6), &Ipv4::SetDown, ipv41, 3);
    //Simulator::Schedule (Seconds(2), &Ipv4::SetDown, ipv41, 2);
    //Simulator::Schedule (Seconds(4), &Ipv4::SetDown, ipv41, 2);
    

  p2p1.EnablePcapAll("multi");
  FlowMonitorHelper flowHelper;
  Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

  Simulator::Stop (Seconds (50));
  Simulator::Run ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ()); 
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (( t.destinationAddress == "9.0.0.2"))
      {
          std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      	  std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstRxPacket.GetSeconds())/1024/1024  << " Mbps\n";
      }
     }
  
  Simulator::Destroy ();
  return 0;
}

   
