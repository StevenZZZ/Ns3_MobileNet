//  n0-------n1-------n2



#include <iostream>

#include <string>
#include <fstream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("tcp_change-bandwidth-2-1");

Ptr<PacketSink> sinks;


// change the bandwidth
void BandwidthTrace1()
{
  Config::Set("/NodeList/1/DeviceList/1/$ns3::PointToPointNetDevice/DataRate", StringValue("8Mbps") );
}
void BandwidthTrace2()
{

  Config::Set("/NodeList/1/DeviceList/1/$ns3::PointToPointNetDevice/DataRate", StringValue("3Mbps") );

}
void BandwidthTrace3()
{ 
  Config::Set("/NodeList/1/DeviceList/1/$ns3::PointToPointNetDevice/DataRate", StringValue("1Mbps") );
}
void BandwidthTrace4()
{
  Config::Set("/NodeList/1/DeviceList/1/$ns3::PointToPointNetDevice/DataRate", StringValue("9Mbps") );
}
void BandwidthTrace5()
{
  Config::Set("/NodeList/1/DeviceList/1/$ns3::PointToPointNetDevice/DataRate", StringValue("2Mbps") );
}
void BandwidthTrace6()
{
  Config::Set("/NodeList/1/DeviceList/1/$ns3::PointToPointNetDevice/DataRate", StringValue("7Mbps") );
}
void BandwidthTrace7()
{
  Config::Set("/NodeList/1/DeviceList/1/$ns3::PointToPointNetDevice/DataRate", StringValue("3Mbps") );
}


// Trace the congestion window
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


int 
main (int argc, char *argv[])
{ // Set Receive Buffer Size large enough
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (600000000));
  NodeContainer c;
  c.Create (3);
   
  DataRate linkRate1("100Mbps");
  DataRate linkRate2("10Mbps");

  // Create the point-to-point link required by the topology
  PointToPointHelper p2p1;
  p2p1.SetDeviceAttribute ("DataRate", DataRateValue(linkRate1));
  p2p1.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2p1.SetChannelAttribute ("Delay", StringValue ("2ms"));

  PointToPointHelper p2p2;
  p2p2.SetDeviceAttribute ("DataRate", DataRateValue(linkRate2));
  p2p2.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2p2.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NodeContainer n0n1 = NodeContainer (c.Get (0), c.Get (1));
  NodeContainer n1n2 = NodeContainer (c.Get (1), c.Get (2));


  NetDeviceContainer device1 = p2p1.Install (n0n1);
  NetDeviceContainer device2 = p2p2.Install (n1n2);
 
  // Install the internet stack on the nodes
  InternetStackHelper internet;
  internet.Install (c);

  TrafficControlHelper tch;
  uint16_t handle = tch.SetRootQueueDisc ("ns3::RedQueueDisc");
    // Add the internal queue used by Red
  tch.AddInternalQueues (handle, 1, "ns3::DropTailQueue", "MaxPackets", UintegerValue (30));
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

// change the bandwidth every 5 seconds
  double i = 5000;
  for (uint32_t j = 0; j < 1; j++){
      Simulator::Schedule (MilliSeconds(1000+10*j*i+0*i) , &BandwidthTrace1);
      Simulator::Schedule (MilliSeconds(1000+10*j*i+1*i) , &BandwidthTrace2);
      Simulator::Schedule (MilliSeconds(1000+10*j*i+2*i) , &BandwidthTrace3);
      Simulator::Schedule (MilliSeconds(1000+10*j*i+3*i) , &BandwidthTrace4);
      Simulator::Schedule (MilliSeconds(1000+10*j*i+4*i) , &BandwidthTrace5);
      Simulator::Schedule (MilliSeconds(1000+10*j*i+5*i) , &BandwidthTrace6);
      Simulator::Schedule (MilliSeconds(1000+10*j*i+6*i) , &BandwidthTrace7);
  }

// Send tcp packets with BulkSendHelper
  uint16_t port = 8080;   // Discard port (RFC 863)

  BulkSendHelper source ("ns3::TcpSocketFactory",InetSocketAddress (ip2.GetAddress (1), port));

  ApplicationContainer apps = source.Install (c.Get(0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (40.0));

    // n0 -> n2
  PacketSinkHelper sink ("ns3::TcpSocketFactory",Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  apps = sink.Install (c.Get(2));
  apps.Start (Seconds (1.0));
  //apps.Stop (Seconds (50.0));
  
  // Trace the congestion window 
  Simulator::Schedule(Seconds(1.0001), &TraceCwnd);
  p2p2.EnablePcapAll("tcp_change-bandwidth-2-1");
  Simulator::Stop (Seconds (40));
  Simulator::Run ();
  
  Simulator::Destroy ();
  return 0;
}

