/* 3 node scenario with finite buffer at intermediate node, Tcp bulk sent flow from sever to client*/
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
#include "ns3/traffic-control-module.h"
using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("FifthScriptExample");

Ptr<PacketSink> sinks;

static void
CwndTracer (Ptr<OutputStreamWrapper>stream, uint32_t oldval, uint32_t newval)
{
  double new_time = Simulator::Now().GetSeconds();
  *stream->GetStream() << new_time << " " << newval << std::endl;
}

static void
TraceCwnd ()
{
  Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper> (&std::cout);
  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndTracer, stream));
}

//uint32_t pktcounter = 0;
//uint32_t oldcounter = 0;


//void ReceivePacket (Ptr<const Packet> packet, const Address &)
//{
//NS_LOG_DEBUG ("Received one packet!");
//pktcounter ++;
//NS_LOG_UNCOND (pktcounter <<" " << Simulator::Now ().GetSeconds ());
//}

//void Throughput()
//{
//  long double  throughput = ((pktcounter - oldcounter)*500*8/0.01/1024/1024);
//  std::cout <<"\n" << throughput;
  
//  oldcounter = pktcounter; 
 // Simulator::Schedule(Seconds (0.01), &Throughput);
//}

//void TxTrace(Ptr<const Packet> p)
//{
// pktcounter ++;
// NS_LOG_UNCOND (pktcounter <<" " << Simulator::Now ().GetSeconds ());
//}


int 
main (int argc, char *argv[])
{
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
   Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (8000000));

   NodeContainer c;
   c.Create (3);
   
   DataRate linkRate1("100Mbps");
   DataRate linkRate2("10Mbps");

  // Create the point-to-point link required by the topology
  PointToPointHelper p2p1;
  p2p1.SetDeviceAttribute ("DataRate", DataRateValue(linkRate1));
  p2p1.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2p1.SetChannelAttribute ("Delay", StringValue ("5ms"));
 // p2p1.SetQueue("ns3::DropTailQueue","MaxPackets",UintegerValue(1));

  PointToPointHelper p2p2;
  p2p2.SetDeviceAttribute ("DataRate", DataRateValue(linkRate2));
  p2p2.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2p2.SetChannelAttribute ("Delay", StringValue ("5ms"));
 // p2p2.SetQueue("ns3::DropTailQueue","MaxPackets",UintegerValue(1));

  NodeContainer n0n1 = NodeContainer (c.Get (0), c.Get (1));
  NodeContainer n1n2 = NodeContainer (c.Get (1), c.Get (2));


  NetDeviceContainer device1 = p2p1.Install (n0n1);
  NetDeviceContainer device2 = p2p2.Install (n1n2);
 
  // Install the internet stack on the nodes
  InternetStackHelper internet;
  internet.Install (c);

 //Ptr<DropTailQueue> queue = DynamicCast<DropTailQueue> (DynamicCast<PointToPointNetDevice> (device2.Get (0))->GetQueue ());
 //queue->SetAttribute ("MaxPackets", UintegerValue (10));

    TrafficControlHelper tch; // finite buffer space at node
  uint16_t handle = tch.SetRootQueueDisc ("ns3::RedQueueDisc");
   //Add the internal queue used by Red
  tch.AddInternalQueues (handle, 1, "ns3::DropTailQueue", "MaxPackets", UintegerValue (1000));
  QueueDiscContainer qdiscs = tch.Install (device2);

  // We've got the "hardware" in place.  Now we need to add IP addresses.
  Ipv4AddressHelper ipv4h1;
  Ipv4AddressHelper ipv4h2;

  ipv4h1.SetBase ("1.0.0.0", "255.0.0.0");
  ipv4h2.SetBase ("2.0.0.0", "255.0.0.0");

  
   Ipv4InterfaceContainer ip1 = ipv4h1.Assign (device1);
   Ipv4InterfaceContainer ip2 = ipv4h2.Assign (device2);

  Ptr<Node> n0 = c.Get (0);
  Ptr<Node> n1 = c.Get (1);
  Ptr<Node> n2 = c.Get (2);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> Routing1 = ipv4RoutingHelper.GetStaticRouting (n0->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing2 = ipv4RoutingHelper.GetStaticRouting (n1->GetObject<Ipv4> ());
  Ptr<Ipv4StaticRouting> Routing3 = ipv4RoutingHelper.GetStaticRouting (n2->GetObject<Ipv4> ());
  
  Routing1->AddNetworkRouteTo (Ipv4Address ("2.0.0.2"), Ipv4Mask ("255.0.0.0"), 1);
  Routing2->AddNetworkRouteTo (Ipv4Address ("2.0.0.2"), Ipv4Mask ("255.0.0.0"), 2);
  Routing2->AddNetworkRouteTo (Ipv4Address ("1.0.0.1"), Ipv4Mask ("255.0.0.0"), 1);
  Routing3->AddNetworkRouteTo (Ipv4Address ("1.0.0.1"), Ipv4Mask ("255.0.0.0"), 1);

      uint16_t port = 9;   // Discard port (RFC 863)
    BulkSendHelper onoff ("ns3::TcpSocketFactory",InetSocketAddress (ip2.GetAddress (1), port));
    //onoff.SetConstantRate (DataRate ("0.1Mbps"));
    //onoff.SetAttribute ("PacketSize", UintegerValue (1500));
    onoff.SetAttribute ("MaxBytes", UintegerValue (0));

    ApplicationContainer apps = onoff.Install (c.Get(0));
    apps.Start (Seconds (2.0));
    apps.Stop (Seconds (30.0));

    // n0 -> n2
    PacketSinkHelper sink ("ns3::TcpSocketFactory",Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
    apps = sink.Install (c.Get(2));
    sinks = StaticCast<PacketSink> (apps.Get (1));
    apps.Start (Seconds (1.0));
    apps.Stop (Seconds (50.0));
   
  Simulator::Schedule(Seconds(2.0001), &TraceCwnd);
 //Config::ConnectWithoutContext("/NodeList/2/ApplicationList/*/$ns3::PacketSink/Rx",MakeCallback (&ReceivePacket));
   //Ptr<BulkSendApplication> app = DynamicCast<BulkSendApplication> (c.Get(0)->GetApplication(0));
  //app->TraceConnectWithoutContext("Tx", MakeCallback(&TxTrace));

  //Simulator::Schedule(Seconds (0.1), &Throughput);
  p2p1.EnablePcapAll("multi");
  FlowMonitorHelper flowHelper;
  Ptr<FlowMonitor> monitor = flowHelper.InstallAll();
  Simulator::Stop (Seconds (1000));
  Simulator::Run ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ()); 
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (( t.destinationAddress == "2.0.0.2"))
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

