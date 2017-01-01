
// topology view
//  n0                n4
//    \              /
//     \            /
//      n2--------n3
//     /            \
//    /              \
//  n1                n5

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("Task1");


static void 
CwndTracer (uint32_t oldval, uint32_t newval)
{
  NS_LOG_INFO ("Moving cwnd from " << oldval << " to " << newval);
}




int 
main (int argc, char *argv[])
{

  CommandLine cmd;
  bool enableFlowMonitor = false;

  cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);

  cmd.Parse (argc, argv);
  
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;


  nodes.Create (6);
  NodeContainer n0n2 = NodeContainer (nodes.Get (0), nodes.Get (2));
  NodeContainer n1n2 = NodeContainer (nodes.Get (1), nodes.Get (2));
  NodeContainer n2n3 = NodeContainer (nodes.Get (2), nodes.Get (3));
  NodeContainer n3n4 = NodeContainer (nodes.Get (3), nodes.Get (4));
  NodeContainer n3n5 = NodeContainer (nodes.Get (3), nodes.Get (5));

  InternetStackHelper internet;
  internet.Install (nodes);

   
  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));
  NetDeviceContainer d0d2 = p2p.Install (n0n2);
  NetDeviceContainer d1d2 = p2p.Install (n1n2);
  NetDeviceContainer d3d4 = p2p.Install (n3n4);
  NetDeviceContainer d3d5 = p2p.Install (n3n5);
  
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay",StringValue ("10ms"));
  NetDeviceContainer d2d3 = p2p.Install (n2n3);

  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i2 = ipv4.Assign (d0d2);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign (d1d2);
   
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i3 = ipv4.Assign (d2d3);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i4 = ipv4.Assign (d3d4);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i5 = ipv4.Assign (d3d5);

  //Create router nodes, initialize routing database and set up the routing
  //tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

   
  // Create the OnOff application to send TCP datagrams of size

  NS_LOG_INFO ("Create Applications.");

  //uint16_t port = 8080;

  uint16_t port = 8080;
  BulkSendHelper onoff ("ns3::TcpSocketFactory",
                         InetSocketAddress (i0i2.GetAddress (0), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  ApplicationContainer apps = onoff.Install (nodes.Get (4));

  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (40.0));



  // Trace changes to the congestion window

  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndTracer));

//
// Create a PacketSinkApplication and install it on node 1
//
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  apps = sink.Install (nodes.Get (0));
  apps.Start (Seconds (1.5));
  apps.Stop (Seconds (40.0)); 

 
  // Create the OnOff application to send UDP datagrams of size



 
  uint16_t port2 = 9;   
  OnOffHelper onoff2 ("ns3::UdpSocketFactory", 
                        Address (InetSocketAddress (i1i2.GetAddress (0), port2)));
  onoff2.SetConstantRate (DataRate ("5Mbps"));
  ApplicationContainer apps2 = onoff2.Install (nodes.Get (5));
  apps2.Start (Seconds (10.0));
  apps2.Stop (Seconds (20.0));

   
  // Create a packet sink to receive these packets
  PacketSinkHelper sink2 ("ns3::UdpSocketFactory",
                            Address (InetSocketAddress (Ipv4Address::GetAny (), port2)));
  apps2 = sink2.Install (nodes.Get (1));
  apps2.Start (Seconds (1.5));
  apps2.Stop (Seconds (20.0));

   



  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("global-routing.tr");
  p2p.EnableAsciiAll (stream);
  p2p.EnablePcapAll ("global-routing");


 
  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("Test-1.tr"));
  p2p.EnablePcapAll ("Test-1");
 

 // Flow Monitor
  FlowMonitorHelper flowmonHelper;
  if (enableFlowMonitor)
    {
      flowmonHelper.InstallAll ();
    }

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (40.0));
  Simulator::Run ();
  NS_LOG_INFO ("Done.");

  if (enableFlowMonitor)
    {
      flowmonHelper.SerializeToXmlFile ("Test-1.flowmon", false, false);
    }

  Simulator::Destroy ();
  return 0;

}
