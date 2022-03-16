/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Universita' di Firenze, Italy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */


// Network topology
//
//      n0                                      n1
//  +---------+                             +--------+
//  | UDP     |             r               | UDP    |
//  +---------+    +---------+--------+     +--------+
//  | IPv6    |    | IPv6    | IPv6   |     | IPv6   |
//  +---------+    +---------+        |     |        |
//  | 6LoWPAN |    | 6LoWPAN |        |     |        |
//  +---------+    +---------+--------+     +--------+
//  | CSMA    |    | CSMA    | CSMA   |     | CSMA   |
//  +---------+    +---------+--------+     +--------+
//       |              |        |               |
//       ================        =================
//
// - Tracing of queues and packet receptions to file "example-sixlowpan.tr"
// Note that the Pcap packet dissection will not be very meaningful.
// See the module's documentation for a discussion about this.

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/sixlowpan-module.h"

/*Test UDP */
#include "ns3/udp-echo-helper.h" //echo bidirezionale

#include "ns3/udp-client-server-helper.h" //unidirezionale
/* */
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ExampleSixlowpan");

int main (int argc, char** argv)
{
  bool verbose = false;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("verbose", "turn on some relevant log components", verbose);
  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("SixLowPanNetDevice", LOG_LEVEL_ALL);
      LogComponentEnable ("Ping6Application", LOG_LEVEL_ALL);
    }

  Packet::EnablePrinting ();
  Packet::EnableChecking ();

  NS_LOG_INFO ("Create nodes.");
  Ptr<Node> n1 = CreateObject<Node> ();
  Ptr<Node> n2 = CreateObject<Node> ();
  Ptr<Node> n3 = CreateObject<Node> ();
  Ptr<Node> n5 = CreateObject<Node> ();
  Ptr<Node> n4 = CreateObject<Node> ();
  Ptr<Node> n6 = CreateObject<Node> ();
  Ptr<Node> n7 = CreateObject<Node> ();
  Ptr<Node> n8 = CreateObject<Node> ();
  Ptr<Node> g = CreateObject<Node> (); //gateway è il nostro router, gli altri sono nodi terminali

  NodeContainer net1(n1, g);
  NodeContainer net2(g, n2);
  NodeContainer net3(n3, g);
  NodeContainer net5(g, n5);
  NodeContainer net4(n4, g);
  NodeContainer net6(g, n6);
  NodeContainer net7(n7, g);
  NodeContainer net8(g, n8);

  NodeContainer all(n1);
  all.Add(n2);
  all.Add(n3);
  all.Add(n4);
  all.Add(n5);
  all.Add(n6);
  all.Add(n7);
  all.Add(n8);
  all.Add(g);

  //NodeContainer all(rete12, rete35, rete46, rete78, g);
  NS_LOG_INFO ("Create IPv6 Internet Stack");
  InternetStackHelper internetv6;
  internetv6.Install (all);
  
  NS_LOG_INFO ("Create channels.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  //NetDeviceContainer d2 = csma.Install (net2);
  csma.SetDeviceAttribute ("Mtu", UintegerValue (150));
  NetDeviceContainer d1 = csma.Install (net1);
  NetDeviceContainer d2 = csma.Install (net2);
  NetDeviceContainer d3 = csma.Install (net3);
  NetDeviceContainer d5 = csma.Install (net5);
  NetDeviceContainer d4 = csma.Install (net4);
  NetDeviceContainer d6 = csma.Install (net6);
  NetDeviceContainer d7 = csma.Install (net7);
  NetDeviceContainer d8 = csma.Install (net8);


  SixLowPanHelper sixlowpan;
  sixlowpan.SetDeviceAttribute ("ForceEtherType", BooleanValue (true) );
  NetDeviceContainer six1 = sixlowpan.Install (d1);
  NetDeviceContainer six2 = sixlowpan.Install (d2);
  NetDeviceContainer six3 = sixlowpan.Install (d3);
  NetDeviceContainer six5 = sixlowpan.Install (d5);
  NetDeviceContainer six4 = sixlowpan.Install (d4);
  NetDeviceContainer six6 = sixlowpan.Install (d6);
  NetDeviceContainer six7 = sixlowpan.Install (d7);
  NetDeviceContainer six8 = sixlowpan.Install (d8);


  NS_LOG_INFO ("Create networks and assign IPv6 Addresses.");
  Ipv6AddressHelper ipv6;
  ipv6.SetBase (Ipv6Address ("2001:1::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i1 = ipv6.Assign (six1);
  i1.SetForwarding (1, true); // i nodi che vanno verso il router 
  i1.SetDefaultRouteInAllNodes (1); // vogliono a 1 , gli altri a zero

  ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i2 = ipv6.Assign (six2);
  i2.SetForwarding (0, true);
  i2.SetDefaultRouteInAllNodes (0);

  ipv6.SetBase (Ipv6Address ("2001:3::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i3 = ipv6.Assign (six3);
  i3.SetForwarding (1, true);
  i3.SetDefaultRouteInAllNodes (1);
 
  ipv6.SetBase (Ipv6Address ("2001:5::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i5 = ipv6.Assign (six5);
  i5.SetForwarding (0, true);
  i5.SetDefaultRouteInAllNodes (0);

  ipv6.SetBase (Ipv6Address ("2001:4::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i4 = ipv6.Assign (six4);
  i4.SetForwarding (1, true);
  i4.SetDefaultRouteInAllNodes (1);
  
  ipv6.SetBase (Ipv6Address ("2001:6::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i6 = ipv6.Assign (six6);
  i6.SetForwarding (0, true);
  i6.SetDefaultRouteInAllNodes (0);

  ipv6.SetBase (Ipv6Address ("2001:7::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i7 = ipv6.Assign (six7);
  i7.SetForwarding (1, true);
  i7.SetDefaultRouteInAllNodes (1);

  ipv6.SetBase (Ipv6Address ("2001:8::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i8 = ipv6.Assign (six8);
  i8.SetForwarding (0, true);
  i8.SetDefaultRouteInAllNodes (0);

  


  

  /* Create a Ping6 application to send ICMPv6 echo request from n0 to n1 via r */
  //uint32_t packetSize = 200;
  /*uint32_t maxPacketCount = 50;
  Time interPacketInterval = Seconds (1.);
  

  Ping6Helper p12;

  p12.SetLocal (i1.GetAddress (0, 1));
  p12.SetRemote (i2.GetAddress (1, 1));

  p12.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  p12.SetAttribute ("Interval", TimeValue (interPacketInterval));
  p12.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer a12 = p12.Install (net1.Get (0));
  a12.Start (Seconds (2));
  a12.Stop (Seconds (4));

  Ping6Helper p35;

  p35.SetLocal (i3.GetAddress (0, 1));
  p35.SetRemote (i5.GetAddress (1, 1));

  p35.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  p35.SetAttribute ("Interval", TimeValue (interPacketInterval));
  p35.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer a35 = p35.Install (net3.Get (0));
  a35.Start (Seconds (5));
  a35.Stop (Seconds (7));

//questo è di più
 Ping6Helper p15;

  p15.SetLocal (i1.GetAddress (0, 1));
  p15.SetRemote (i5.GetAddress (1, 1));

  p15.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  p15.SetAttribute ("Interval", TimeValue (interPacketInterval));
  p15.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer a15 = p15.Install (net1.Get (0));
  a15.Start (Seconds (8));
  a15.Stop (Seconds (11));

  Ping6Helper p46;

  p46.SetLocal (i4.GetAddress (0, 1));
  p46.SetRemote (i6.GetAddress (1, 1));

  p46.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  p46.SetAttribute ("Interval", TimeValue (interPacketInterval));
  p46.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer a46 = p46.Install (net4.Get (0));
  a46.Start (Seconds (5)); //test due insieme
  a46.Stop (Seconds (7));
  a46.Start (Seconds (11));
  a46.Stop (Seconds (12));// 

  Ping6Helper p78;

  p78.SetLocal (i7.GetAddress (0, 1));
  p78.SetRemote (i8.GetAddress (1, 1));

  p78.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  p78.SetAttribute ("Interval", TimeValue (interPacketInterval));
  p78.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer a78 = p78.Install (net7.Get (0));
  a78.Start (Seconds (13));
  a78.Stop (Seconds (15));
*/


/* Udp echo 
  UdpEchoServerHelper server = UdpEchoServerHelper(5000);
  ApplicationContainer ap2= server.Install(net1.Get(1));
  ap2.Start(Seconds(2));
  ap2.Stop(Seconds(15));

  UdpEchoClientHelper client= UdpEchoClientHelper(i1.GetAddress(1,1), 5000);
  ApplicationContainer ap=client.Install(net1.Get(0));
  ap.Start(Seconds(3));
  ap.Stop(Seconds(12));
  
  ApplicationContainer ap3=client.Install(net3.Get(0));
  ap3.Start(Seconds(3));
  ap3.Stop(Seconds(12));

  ApplicationContainer ap7=client.Install(net7.Get(0));
  ap7.Start(Seconds(3));
  ap7.Stop(Seconds(12));
*/


// udp unidirezionale
  UdpServerHelper server = UdpServerHelper(5000);
  ApplicationContainer ap2= server.Install(net1.Get(1));
  ap2.Start(Seconds(2));
  ap2.Stop(Seconds(15));

  UdpClientHelper client= UdpClientHelper(i1.GetAddress(1,1), 5000);
  ApplicationContainer ap=client.Install(net1.Get(0));
  ap.Start(Seconds(3));
  ap.Stop(Seconds(12));

  ApplicationContainer ap3=client.Install(net3.Get(0));
  ap3.Start(Seconds(3));
  ap3.Stop(Seconds(12));

  ApplicationContainer ap7=client.Install(net7.Get(0));
  ap7.Start(Seconds(3));
  ap7.Stop(Seconds(12));

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("example-sixlowpan.tr"));
  csma.EnablePcapAll (std::string ("example-sixlowpan"), true);

  Simulator::Stop (Seconds (100));
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
