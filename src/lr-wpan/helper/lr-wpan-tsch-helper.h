/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 The Boeing Company
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
 * Authors:
 *  Gary Pei <guangyu.pei@boeing.com>
 *  Tom Henderson <thomas.r.henderson@boeing.com>
 *  Luis Pacheco <luisbelem@gmail.com>
 *  Peishuo Li <pressthunder@gmail.com>
 */
#ifndef LR_WPAN_TSCH_HELPER_H
#define LR_WPAN_TSCH_HELPER_H

#include <vector>
#include <ns3/node-container.h>
#include <ns3/lr-wpan-phy.h>
#include <ns3/lr-wpan-radio-energy-model.h>
#include <ns3/lr-wpan-energy-source.h>
#include <ns3/lr-wpan-tsch-mac.h>
#include <ns3/lr-wpan-tsch-net-device.h>
#include <ns3/spectrum-channel.h>
#include <ns3/trace-helper.h>
#include "ns3/energy-module.h"
#include <ns3/lr-wpan-array.h>
#include <ns3/random-variable-stream.h>

namespace ns3 {

class MobilityModel;

/**
 * \ingroup lr-wpan
 *
 * \brief helps to manage and create IEEE 802.15.4 NetDevice objects
 *
 * This class can help to create IEEE 802.15.4 NetDevice objects
 * and to configure their attributes during creation.  It also contains
 * additional helper functions used by client code.
 */


struct AddLinkParams {
  uint8_t slotframeHandle;
  uint16_t linkHandle;
  uint16_t timeslot;
  uint8_t channelOffset;
};

class LrWpanTschHelper : public PcapHelperForDevice,
                     public AsciiTraceHelperForDevice
{
public:
  /**
   * @brief LrWpanTschHelper: Create a LrWpan helper in an empty state.
   */
  LrWpanTschHelper (void);
  LrWpanTschHelper (Ptr<SpectrumChannel> ch);
  LrWpanTschHelper (Ptr<SpectrumChannel> ch, u_int32_t num_node, bool fadingBiasMatrix, bool isDay);
  virtual ~LrWpanTschHelper (void);

  /**
  * @brief AddMobility: Add mobility model to a physical device
  * @param phy: the physical device
  * @param m: the mobility model
  */
  void AddMobility (Ptr<LrWpanPhy> phy, Ptr<MobilityModel> m);

  /**
   * @brief Install: Install TSCH netdevice and Friis propagation channel to NodeContainer
   * @param c: a set of nodes
   * @return TSCH stack in the form of NetDeviceContainer
   */
  NetDeviceContainer Install (NodeContainer c);

  /**
   * @brief InstallVector: Install TSCH netdevice and Friis propagation channel to NodeContainer
   * @param c: a set of nodes
   * @return TSCH stack in the form of LrWpanTschNetDevice pointer vector
   */
  std::vector<Ptr<LrWpanTschNetDevice> > InstallVector (NodeContainer c);

  /**
   * @brief InstallEnergySource: Install LrWpan EnergySource to NodeContainer
   * @param c: a set of nodes
   * @return LrWpan EnergySource stack
   */
  EnergySourceContainer InstallEnergySource (NodeContainer c);

  /**
   * @brief InstallEnergyDevice: Install LrWpan TSCH DeviceEnergyModel to NetDeviceContainer attached with given energy sources
   * @param devices: a set of netdevice
   * @param sources: a set of energy source on the node
   * @return LrWpan TSCH DeviceEnergyModel stack
   */
  DeviceEnergyModelContainer InstallEnergyDevice (NetDeviceContainer devices, EnergySourceContainer sources);

  /**
   * @brief AssociateToPan: Associate the nodes to the same PAN
   * @param c: a set of nodes
   * @param panId: the PAN Id
   */
  void AssociateToPan (NetDeviceContainer c, uint16_t panId);

  /**
   * Helper to enable all LrWpan log components with one statement
   */
  void EnableLogComponents (void);

  /**
   * @brief LrWpanPhyEnumerationPrinter: print transceiver PHY states
   */
  static std::string LrWpanPhyEnumerationPrinter (LrWpanPhyEnumeration);

  /**
   * @brief LrWpanMacStatePrinter: print link MAC states
   */
  static std::string LrWpanMacStatePrinter (LrWpanTschMacState e);

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model. Return the number of streams that have been
   * assigned. The Install() method should have previously been
   * called by the user.
   *
   * @param c: NetDeviceContainer of the set of net devices for which the
   *          CsmaNetDevice should be modified to use a fixed stream
   * @param stream: first stream index to use
   * @return: the number of stream indices assigned by this helper
   */
  int64_t AssignStreams (NetDeviceContainer c, int64_t stream);

  /**
   * @brief AddSlotframe
   * @param dev: pointer of one netdevice
   * @param slotframehandle
   * @param size
   */
  void AddSlotframe(NetDeviceContainer nodes, uint8_t slotframehandle, uint16_t size);
  void AddSlotframe(Ptr<NetDevice> dev, uint8_t slotframehandle, uint16_t size);


  /**
   * @brief ModSlotframe
   * @param nodes: a set of netdevices
   * @param slotframehandle
   * @param size
   */
  void ModSlotframe(NetDeviceContainer nodes, uint8_t slotframehandle, uint16_t size);

  /**
   * @brief AddLink
   * @param src
   * @param dst
   * @param params: slotframe handle, link handle, timeslot number, channel offset
   */
  void AddLink(Ptr<NetDevice> src, Ptr<NetDevice> dst, AddLinkParams params);  
  void AddLink(NetDeviceContainer devs, u_int32_t srcPos, u_int32_t dstPos, AddLinkParams params, bool sharedLink=false);

  /**
   * @brief ModifyLink
   * @param src
   * @param dst
   * @param params: slotframe handle, link handle, timeslot number, channel offset
   */
  void ModifyLink(Ptr<NetDevice> src, Ptr<NetDevice> dst, AddLinkParams params);
  void ModifyLink(NetDeviceContainer devs, u_int32_t srcPos, u_int32_t dstPos, AddLinkParams params, bool sharedLink);

  /**
   * @brief DeleteLink
   * @param src
   * @param dst
   * @param params: slotframe handle, link handle, timeslot number, channel offset
   */
  void DeleteLink(Ptr<NetDevice> src, Ptr<NetDevice> dst, AddLinkParams params);
  void DeleteLink(NetDeviceContainer devs, u_int32_t srcPos, u_int32_t dstPos, AddLinkParams params, bool sharedLink);

  /**
   * @brief ConfigureSlotframeAllToPan: configure slotframe and links
   * Automatically creates a star network with
   * one timeslot for each node to send a packet to
   * the PAN coordinator (which is the first node)
   * Empty timeslots can also be defined
   * @param devs: a set of netdevices
   * @param empty_timeslots
   */
  void ConfigureSlotframeAllToPan(NetDeviceContainer devs, int empty_timeslots, bool bidir, bool bcast);

  /**
   * @brief EnableTsch: activate and deactivate TSCH
   * @param devs
   * @param start
   * @param duration
   */
  void EnableTsch(NetDeviceContainer devs, double start, double duration);

  /**
   * @brief EnableEnergyAll: tracing energy for all devices of each node based on MAC timeslot type
   * @param stream: output stream to certain file
   */
  void EnableEnergyAll(Ptr<OutputStreamWrapper> stream);

  /**
   * @brief EnableEnergyAll: tracing energy for certain device based on different MAC timeslot types
   * @param stream: output stream to certain file
   * @param dev
   */
  void EnableEnergy(Ptr<OutputStreamWrapper> stream, Ptr<NetDevice> dev);

  /**
   * @brief EnableEnergyAllPhy: tracing energy for all devices of each node based on different traceiver PHY states
   * @param stream: output stream to certain file
   * @param sources
   */
  void EnableEnergyAllPhy(Ptr<OutputStreamWrapper> stream, EnergySourceContainer sources);

  /**
   * @brief GenerateTraffic: Generate CBR traffic for given devices
   * @param dev
   * @param dst
   * @param packet_size
   * @param start
   * @param duration
   * @param interval
   */
  void GenerateTraffic(Ptr<NetDevice> dev, Address dst, int packet_size, double start, double duration, double interval);

  /**
   * @brief SendPacket
   * @param dev
   * @param dst
   * @param packet_size
   * @param interval
   * @param end
   */
  void SendPacket(Ptr<NetDevice> dev, Address dst, int packet_size, double interval,double end);

  /**
   * @brief Add an advertising link
   * @param all devices
   * @param position of the sender in the device container
   * @param link params
   */
  void AddAdvLink(NetDeviceContainer devs,u_int32_t senderPos, AddLinkParams params);

  /**
   * @brief Add a broadcast link
   * @param all devices
   * @param position of the receiver in the device container
   * @param link params
   */
  void AddBcastLinks(NetDeviceContainer devs,u_int32_t coordinatorPos, AddLinkParams params);

  /**
   * @brief Set the bias coefficients which describe the multi-path effect on the current channel
   */
  void SetFadingBiasValues();

  /**
   * @brief Print the bias coefficients which describe the multi-path effect on the current channel
   */
  void PrintFadingBiasValues(Ptr<OutputStreamWrapper> stream_fadingBias);

  void EnableReceivePower (Ptr<OutputStreamWrapper> stream_recPower, NodeContainer lrwpanNodes);

private:
  // Disable implicit constructors
  LrWpanTschHelper (LrWpanTschHelper const &);
  LrWpanTschHelper& operator= (LrWpanTschHelper const &);
  /**
   * \internal
   *
   * \brief Enable pcap output on the indicated net device.
   * \internal
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param prefix Filename prefix to use for pcap files.
   * \param nd Net device for which you want to enable tracing.
   * \param promiscuous If true capture all possible packets available at the device.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename);

  /**
   * \brief Enable ascii trace output on the indicated net device.
   * \internal
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param stream The output stream object to use when logging ascii traces.
   * \param prefix Filename prefix to use for ascii trace files.
   * \param nd Net device for which you want to enable tracing.
   */
  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream,
                                    std::string prefix,
                                    Ptr<NetDevice> nd,
                                    bool explicitFilename);

  /**
   * @brief EnableEnergyInternal: tracing energy for certain node based on MAC timeslot type
   * \param stream The output stream object to use when logging energy traces.
   * \param prefix Filename prefix to use for ascii energy files.
   * \param nd Net device for which you want to enable tracing.
   * \param explicitFilename Treat the prefix as an explicit filename if true
   */
  void EnableEnergyInternal (Ptr<OutputStreamWrapper> stream,
                             std::string prefix,
                             Ptr<NetDevice> nd,
                             bool explicitFilename);

  Ptr<SpectrumChannel> m_channel;               // channel model
  int m_slotframehandle;                        // slotframe handle
  u_int32_t m_numchannel;                       // number of TSCH channels, default 16
  u_int32_t m_numnode;                          // number of lrwpan nodes
  LrWpanArray3D<double> FadingBias;             // 3D bias coefficient matrix which describe the multi-path effect of specific link on specific channel
  Ptr<UniformRandomVariable> m_random;          // random variable to set the 3D matrix values
  double MinFadingBias;
  double MaxFadingBias;
  bool m_fadingBiasMatrix;
  bool m_isDay;
};

}

#endif /* LR_WPAN_HELPER_H */
