/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 CTTC
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 * Author: Luis Pacheco <luisbelem@gmail.com>
 * Author: Peter Kourzanov <peter.kourzanov@gmail.com>
 */
#include <iostream>

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/spectrum-model-ism2400MHz-res1MHz.h>
#include <ns3/spectrum-model-300kHz-300GHz-log.h>
#include <ns3/wifi-spectrum-value-helper.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/waveform-generator.h>
#include <ns3/spectrum-analyzer.h>
#include <ns3/spectrum-converter.h>
#include <ns3/log.h>
#include <string>
#include <ns3/friis-spectrum-propagation-loss.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/mobility-module.h>
#include <ns3/spectrum-helper.h>
#include <ns3/applications-module.h>
#include <ns3/adhoc-aloha-noack-ideal-phy-helper.h>
#include <ns3/waveform-generator-helper.h>
#include <ns3/spectrum-analyzer-helper.h>
#include <ns3/non-communicating-net-device.h>


#include "ns3/internet-module.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/sixlowpan-module.h"

NS_LOG_COMPONENT_DEFINE ("TschTest");

using namespace ns3;
using namespace std;

/////////////////////////////////
// Configuration
/////////////////////////////////
int pktsize = 60;           //size of packets, in bytes
int nrnodes = 1;             //number of nodes, not including the coordinator
double duty_cycle = 0.1;      //proportion of assigned timeslots with actual packets to send
double duration = 20;       //simulation total duration, in seconds
bool verbose = false;       //enable logging (different from trace)
bool interference = false;  //enable wifi interference

/////////////////////////////////
// End configuration
/////////////////////////////////
void SingleWifiPacket(NodeContainer ofdmNodes, Ptr<SpectrumChannel> channel)
{
  WifiSpectrumValue5MhzFactory sf;
  double txPower = 0.1; // Watts
  WaveformGeneratorHelper waveformGeneratorHelper;
  Ptr<SpectrumValue> txPsd = sf.CreateTxPowerSpectralDensity (txPower, 6);


  waveformGeneratorHelper.SetTxPowerSpectralDensity (txPsd);
  waveformGeneratorHelper.SetChannel (channel);
  waveformGeneratorHelper.SetPhyAttribute ("Period", TimeValue (Seconds (1)));
  waveformGeneratorHelper.SetPhyAttribute ("DutyCycle", DoubleValue (0.00022637));
  NetDeviceContainer waveformGenerator0 = waveformGeneratorHelper.Install (ofdmNodes.Get(0));

  //Data
  Simulator::Schedule (MicroSeconds(101.5), &WaveformGenerator::Start, waveformGenerator0.Get (0)->GetObject<NonCommunicatingNetDevice> ()->GetPhy ()->GetObject<WaveformGenerator> ());
  Simulator::Schedule (MicroSeconds(101.5+226.37), &WaveformGenerator::Stop, waveformGenerator0.Get (0)->GetObject<NonCommunicatingNetDevice> ()->GetPhy ()->GetObject<WaveformGenerator> ());

  waveformGeneratorHelper.SetPhyAttribute ("Period", TimeValue (Seconds (1)));
  waveformGeneratorHelper.SetPhyAttribute ("DutyCycle", DoubleValue (0.00002207));
  NetDeviceContainer waveformGenerator1 = waveformGeneratorHelper.Install (ofdmNodes.Get(1));

  //ACK
  Simulator::Schedule (MicroSeconds(101.5+226.37+16), &WaveformGenerator::Start, waveformGenerator1.Get (0)->GetObject<NonCommunicatingNetDevice> ()->GetPhy ()->GetObject<WaveformGenerator> ());
  Simulator::Schedule (MicroSeconds(101.5+226.37+16+22.07), &WaveformGenerator::Stop, waveformGenerator1.Get (0)->GetObject<NonCommunicatingNetDevice> ()->GetPhy ()->GetObject<WaveformGenerator> ());

  Simulator::Schedule (MicroSeconds(101.5+226.37+16+22.07), &SingleWifiPacket, ofdmNodes, channel);
}

void EnableTsch(LrWpanTschHelper* lrWpanHelper,NetDeviceContainer& netdev)
{
  lrWpanHelper->EnableTsch(netdev,0,duration);
}

/**
 * \class StackHelper
 * \brief Helper to set or get some IPv6 information about nodes.
 */
class StackHelper
{
public:

  /**
   * \brief Add an address to a IPv6 node.
   * \param n node
   * \param interface interface index
   * \param address IPv6 address to add
using namespace std;
   */
  inline void AddAddress (Ptr<Node>& n, uint32_t interface, Ipv6Address address)
  {
    Ptr<Ipv6> ipv6 = n->GetObject<Ipv6> ();
    ipv6->AddAddress (interface, address);
  }

  /**
   * \brief Print the routing table.
   * \param n the node
   */
  inline void PrintRoutingTable (Ptr<Node>& n)
  {
    Ptr<Ipv6StaticRouting> routing = 0;
    Ipv6StaticRoutingHelper routingHelper;
    Ptr<Ipv6> ipv6 = n->GetObject<Ipv6> ();
    uint32_t nbRoutes = 0;
    Ipv6RoutingTableEntry route;

    routing = routingHelper.GetStaticRouting (ipv6);

    cout << "Routing table of " << n << " : " << endl;
    cout << "Destination\t\t\t\t" << "Gateway\t\t\t\t\t" << "Interface\t" <<  "Prefix to use" << endl;

    nbRoutes = routing->GetNRoutes ();
    for (uint32_t i = 0; i < nbRoutes; i++)
      {
        route = routing->GetRoute (i);
        cout << route.GetDest () << "\t"
                  << route.GetGateway () << "\t"
                  << route.GetInterface () << "\t"
                  << route.GetPrefixToUse () << "\t"
                  << endl;
      }
  }
};

int main (int argc, char** argv)
{
  CommandLine cmd;
  cmd.AddValue ("verbose", "Print trace information if true", verbose);
  cmd.Parse (argc, argv);
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  //Enable PCAP and Ascii Tracing
  AsciiTraceHelper ascii;

  //Schedule wifi device
  NodeContainer ofdmNodes;
  NodeContainer panCoord;
  NodeContainer sensors;
  NodeContainer allNodes;

  panCoord.Create (1);
  sensors.Create(nrnodes);
  NodeContainer lrwpanNodes(panCoord,sensors);
  ofdmNodes.Create (1);
  allNodes.Add (lrwpanNodes);
  allNodes.Add (ofdmNodes);

  /////////////////////////////////
  // Mobility
  /////////////////////////////////

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "GridWidth", UintegerValue(4),
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5),
                                 "DeltaY", DoubleValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (allNodes);

  /////////////////////////////////
  // Upper layers
  /////////////////////////////////
  //IPv6
  InternetStackHelper internetv6;
  internetv6.SetIpv4StackInstall (false);
  internetv6.Install (lrwpanNodes);
  /////////////////////////////////
  // Channel
  /////////////////////////////////

  SpectrumChannelHelper channelHelper = SpectrumChannelHelper::Default ();
  channelHelper.SetChannel ("ns3::MultiModelSpectrumChannel");
  Ptr<SpectrumChannel> channel = channelHelper.Create ();

  /////////////////////////////////
  // Configure lrwpan nodes
  /////////////////////////////////

  LrWpanTschHelper lrWpanHelper(channel,nrnodes+1,false,true);
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("lr-wpan-tsch.tr");
  Ptr<OutputStreamWrapper> fstream = ascii.CreateFileStream ("lr-wpan-tsch.fading");
  lrWpanHelper.PrintFadingBiasValues(fstream);
  NetDeviceContainer netdev = lrWpanHelper.Install (lrwpanNodes);

  lrWpanHelper.EnablePcapAll (string ("lr-wpan-tsch"), true);
  lrWpanHelper.EnableAsciiAll (stream);

  lrWpanHelper.AssociateToPan(netdev,123);
  lrWpanHelper.ConfigureSlotframeAllToPan(netdev,0,true,false);//devices,empty slots,bidirectional=true,use broadcast cells=false
  Simulator::Schedule(Seconds(5),&EnableTsch,&lrWpanHelper,netdev);

  /////////////////////////////////
  // Upper layers
  /////////////////////////////////
  //Sixlowpan stack
  StackHelper stackHelper;
  SixLowPanHelper sixlowpan;
  NetDeviceContainer devices = sixlowpan.Install (netdev);

  Ipv6AddressHelper ipv6;
  ipv6.SetBase (Ipv6Address("2001:1::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer deviceInterfaces = ipv6.Assign (devices);

  for (int i = 1;i<=nrnodes;i++) {
	  deviceInterfaces.SetForwarding(i,true);
	  deviceInterfaces.SetDefaultRoute(i,0);
  }

  for (int i=0;i<=nrnodes;i++) {
	Ptr<Node> n=lrwpanNodes.Get(i);
  	cout << "=== node " << deviceInterfaces.GetAddress(i,0) 
	     << " (IEEE " << DynamicCast<LrWpanTschNetDevice>(netdev.Get(i))->GetMac()->GetShortAddress() << ")"
	     << " ===" << endl;
	stackHelper.PrintRoutingTable (n);
  }

  //Ping6
  Ping6Helper ping6;

  ping6.SetAttribute ("MaxPackets", UintegerValue (duration/(0.01*nrnodes*duty_cycle)));
  ping6.SetAttribute ("Interval", TimeValue (Seconds(0.01*nrnodes/duty_cycle)));
  ping6.SetAttribute ("PacketSize", UintegerValue (pktsize));

  ApplicationContainer apps;

  for (int i=1;i<=nrnodes;i++)
    {
      ping6.SetLocal (deviceInterfaces.GetAddress (i, 1));
      ping6.SetRemote (deviceInterfaces.GetAddress (0, 1));
      apps.Add (ping6.Install (lrwpanNodes.Get (i)));
    }
  for (int i=1; i<=nrnodes; i++) {
  	apps.Get(i-1)->SetStartTime (Seconds (i-1));
  }
  apps.Stop (Seconds (duration));

  if (interference) SingleWifiPacket(ofdmNodes, channel);

  /////////////////////////////////
  // Start and finish the simulation
  /////////////////////////////////
  Simulator::Run ();
  Simulator::Destroy ();
}

