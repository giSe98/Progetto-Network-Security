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

#include "ns3/applications-module.h"

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
  Ptr<Node> n0 = CreateObject<Node> ();
  Ptr<Node> r = CreateObject<Node> ();
  Ptr<Node> n1 = CreateObject<Node> ();
  /*1*/  
  Ptr<Node> n2 = CreateObject<Node> ();
  Ptr<Node> n3 = CreateObject<Node> ();
  /**/
  

  NodeContainer net1 (n0, r);
  NodeContainer net2 (r, n1);
  /*2*/
  NodeContainer net3 (r, n2);
  NodeContainer net4 (n3, r);
  /**/
  NodeContainer all (n0, r, n1, n2, n3);


  NS_LOG_INFO ("Create IPv6 Internet Stack");
  InternetStackHelper internetv6;
  internetv6.Install (all);

  NS_LOG_INFO ("Create channels.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer d2 = csma.Install (net2);
  csma.SetDeviceAttribute ("Mtu", UintegerValue (150));
  NetDeviceContainer d1 = csma.Install (net1);

  /*3*/
  NetDeviceContainer d3 = csma.Install (net3);
  NetDeviceContainer d4 = csma.Install (net4);
  /**/

  SixLowPanHelper sixlowpan;
  sixlowpan.SetDeviceAttribute ("ForceEtherType", BooleanValue (true) );
  NetDeviceContainer six1 = sixlowpan.Install (d1);

  /*4*/
  NetDeviceContainer six2 = sixlowpan.Install (d2);
  NetDeviceContainer six3 = sixlowpan.Install (d3);
  NetDeviceContainer six4 = sixlowpan.Install (d4);
  /**/

  NS_LOG_INFO ("Create networks and assign IPv6 Addresses.");
  Ipv6AddressHelper ipv6;
  ipv6.SetBase (Ipv6Address ("2001:1::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i1 = ipv6.Assign (six1);
  i1.SetForwarding (1, true);
  i1.SetDefaultRouteInAllNodes (1);

  ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i2 = ipv6.Assign (six2);
  i2.SetForwarding (0, true);
  i2.SetDefaultRouteInAllNodes (0);

  /*5*/
  ipv6.SetBase (Ipv6Address ("2001:3::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i3 = ipv6.Assign (six3);
  i3.SetForwarding (0, true);
  i3.SetDefaultRouteInAllNodes (0);

  ipv6.SetBase (Ipv6Address ("2001:4::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer i4 = ipv6.Assign (six4);
  i4.SetForwarding (0, false);
  i4.SetDefaultRouteInAllNodes (0);
  /**/


  /* Create a Ping6 application to send ICMPv6 echo request from n0 to n1 via r */
  uint32_t packetSize = 200;
  uint32_t maxPacketCount = 50;
  Time interPacketInterval = Seconds (1.);
  Ping6Helper ping6;

  ping6.SetLocal (i1.GetAddress (0, 1));
  ping6.SetRemote (i2.GetAddress (1, 1));

  ping6.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  ping6.SetAttribute ("Interval", TimeValue (interPacketInterval));
  ping6.SetAttribute ("PacketSize", UintegerValue (packetSize));

  /*6*/
  Ping6Helper ping7;
  ping7.SetLocal (i3.GetAddress (0, 0));
  ping7.SetRemote (i4.GetAddress (1, 0));
  ping7.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  ping7.SetAttribute ("Interval", TimeValue (interPacketInterval));
  ping7.SetAttribute ("PacketSize", UintegerValue (packetSize));
  /**/
  ApplicationContainer apps = ping6.Install (net1.Get (0));
  /*7*/
  ApplicationContainer apps2 = ping7.Install (net3.Get (0));
  /**/
  apps.Start (Seconds (5.0));
  apps.Stop (Seconds (15.0));
  /*8*/
  apps2.Start (Seconds (5.0));
  apps2.Stop (Seconds (11.0));
  /**/

/*
UdpServerHelper server (4000);
//ApplicationContainer ap= server.Install(net3.Get(0));
ApplicationContainer apps = server.Install (net1.Get (0));
apps.Start(Seconds(1.0));
apps.Stop(Seconds(15.0));

UdpClientHelper client(i1.GetAddress(0,1), 4000);
apps=client.Install(net1.Get(0));
apps.Start(Seconds(2.0));
apps.Stop(Seconds(15.0));

  */
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("example-sixlowpan.tr"));
  csma.EnablePcapAll (std::string ("example-sixlowpan"), true);

  Simulator::Stop (Seconds (100));
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

}
