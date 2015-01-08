/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author:
 *  Tom Henderson <thomas.r.henderson@boeing.com>
 *  Tommaso Pecorella <tommaso.pecorella@unifi.it>
 *  Margherita Filippetti <morag87@gmail.com>
 *  Peter Kourzanov <peter.kourzanov@gmail.com>
 */
#include <cassert>
#include "lr-wpan-tsch-net-device.h"
#include "lr-wpan-phy.h"
#include "lr-wpan-csmaca.h"
#include "lr-wpan-error-model.h"
#include <ns3/abort.h>
#include <ns3/node.h>
#include <ns3/log.h>
#include <ns3/spectrum-channel.h>
#include <ns3/pointer.h>
#include <ns3/boolean.h>
#include <ns3/mobility-model.h>
#include <ns3/packet.h>


NS_LOG_COMPONENT_DEFINE ("LrWpanTschNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LrWpanTschNetDevice);

TypeId
LrWpanTschNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LrWpanTschNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<LrWpanTschNetDevice> ()
    .AddAttribute ("Channel", "The channel attached to this device",
                   PointerValue (),
                   MakePointerAccessor (&LrWpanTschNetDevice::DoGetChannel),
                   MakePointerChecker<SpectrumChannel> ())
    .AddAttribute ("Phy", "The PHY layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&LrWpanTschNetDevice::GetPhy,
                                        &LrWpanTschNetDevice::SetPhy),
                   MakePointerChecker<LrWpanPhy> ())
    .AddAttribute ("Mac", "The MAC layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&LrWpanTschNetDevice::GetMac,
                                        &LrWpanTschNetDevice::SetMac),
                   MakePointerChecker<LrWpanTschMac> ())
    .AddAttribute ("UseAcks", "Request acknowledgments for data frames.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&LrWpanTschNetDevice::m_useAcks),
                   MakeBooleanChecker ())
  ;
  return tid;
}

LrWpanTschNetDevice::LrWpanTschNetDevice ()
  : m_isTsch(-1),m_configComplete (false)
{
  NS_LOG_FUNCTION (this);
  m_mac = CreateObject<LrWpanTschMac> ();
  m_omac = CreateObject<LrWpanMac> ();
  m_phy = CreateObject<LrWpanPhy> ();
  m_csmaca = CreateObject<LrWpanCsmaCa> ();
  CompleteConfig ();
}

LrWpanTschNetDevice::~LrWpanTschNetDevice ()
{
  NS_LOG_FUNCTION (this);
}


void
LrWpanTschNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_omac->Dispose ();
  m_mac->Dispose ();
  m_phy->Dispose ();
  m_csmaca->Dispose ();
  m_phy = 0;
  m_mac = 0;
  m_omac = 0;
  m_csmaca = 0;
  m_node = 0;
  // chain up.
  NetDevice::DoDispose ();

}

void
LrWpanTschNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_phy->Initialize ();
  m_mac->Initialize ();
  m_omac->Initialize ();
  NetDevice::DoInitialize ();
}


void
LrWpanTschNetDevice::CompleteConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (m_mac == 0
      || m_omac == 0
      || m_csmaca == 0
      || m_phy == 0
      || m_node == 0
      || m_configComplete)
    {
      return;
    }

  m_mac->SetPhy (m_phy);
  m_omac->SetPhy (m_phy);
  m_omac->SetCsmaCa (m_csmaca);
  m_mac->SetMcpsDataIndicationCallback (MakeCallback (&LrWpanTschNetDevice::McpsDataIndication, this));
  m_omac->SetMcpsDataIndicationCallback (MakeCallback (&LrWpanTschNetDevice::McpsDataIndication, this));
  m_csmaca->SetMac (m_omac);

  m_phy->SetMobility (m_node->GetObject<MobilityModel> ());
  Ptr<LrWpanErrorModel> model = CreateObject<LrWpanErrorModel> ();
  m_phy->SetErrorModel (model);
  m_phy->SetDevice (this);

  // TSCH MAC specific callbacks
  m_mac->SetMlmeSetSlotframeConfirmCallback(MakeCallback (&LrWpanTschNetDevice::SlotframeConfirm,this));
  m_mac->SetMlmeSetLinkConfirmCallback(MakeCallback (&LrWpanTschNetDevice::LinkConfirm,this));
  m_mac->SetMlmeTschModeConfirmCallback(MakeCallback (&LrWpanTschNetDevice::ModeConfirm,this));

  m_csmaca->SetLrWpanMacStateCallback (MakeCallback (&LrWpanMac::SetLrWpanMacState, m_omac));

  // the rest is done here
  SetTschMode(false);
  m_configComplete = true;
}

void
LrWpanTschNetDevice::SetMac (Ptr<LrWpanTschMac> mac)
{
  NS_LOG_FUNCTION (this);
  m_mac = mac;
  CompleteConfig ();
}

void
LrWpanTschNetDevice::SetPhy (Ptr<LrWpanPhy> phy)
{
  NS_LOG_FUNCTION (this);
  m_phy = phy;
  CompleteConfig ();
}

void
LrWpanTschNetDevice::SetCsmaCa (Ptr<LrWpanCsmaCa> csmaca)
{
  NS_LOG_FUNCTION (this);
  m_csmaca = csmaca;
  CompleteConfig ();
}

void
LrWpanTschNetDevice::SetChannel (Ptr<SpectrumChannel> channel)
{
  NS_LOG_FUNCTION (this << channel);
  m_phy->SetChannel (channel);
  channel->AddRx (m_phy);
  CompleteConfig ();
}

Ptr<LrWpanTschMac>
LrWpanTschNetDevice::GetNMac (void) const
{
  // NS_LOG_FUNCTION (this);
  //assert(m_isTsch==true);
  return m_mac;
}

Ptr<LrWpanMac>
LrWpanTschNetDevice::GetOMac (void) const
{
  // NS_LOG_FUNCTION (this);
  //assert(m_isTsch==false);
  //if (m_isTsch) return m_mac;
  return m_omac;
}

Ptr<LrWpanMac>
LrWpanTschNetDevice::GetMac (void) const
{
  // NS_LOG_FUNCTION (this);
  assert(m_isTsch>=0);
  if (m_isTsch) return m_mac;
  return m_omac;
}

Ptr<LrWpanPhy>
LrWpanTschNetDevice::GetPhy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

Ptr<LrWpanCsmaCa>
LrWpanTschNetDevice::GetCsmaCa (void) const
{
  NS_LOG_FUNCTION (this);
  return m_csmaca;
}

void
LrWpanTschNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}

uint32_t
LrWpanTschNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}

Ptr<Channel>
LrWpanTschNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy->GetChannel ();
}

void
LrWpanTschNetDevice::LinkUp (void)
{
  NS_LOG_FUNCTION (this);
  m_linkUp = true;
  m_linkChanges ();
}

void
LrWpanTschNetDevice::LinkDown (void)
{
  NS_LOG_FUNCTION (this);
  m_linkUp = false;
  m_linkChanges ();
}

Ptr<SpectrumChannel>
LrWpanTschNetDevice::DoGetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy->GetChannel ();
}

void
LrWpanTschNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this);
  assert(m_isTsch>=0);
  if (m_isTsch)
	  m_mac->SetShortAddress (Mac16Address::ConvertFrom (address));
  else
	  m_omac->SetShortAddress (Mac16Address::ConvertFrom (address));
}

Address
LrWpanTschNetDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION (this);
  assert(m_isTsch>=0);
  if ( m_isTsch)
	  return m_mac->GetShortAddress ();
  else 
	  return m_omac->GetShortAddress ();
}

bool
LrWpanTschNetDevice::SetMtu (const uint16_t mtu)
{
  NS_ABORT_MSG ("Unsupported");
  return false;
}

uint16_t
LrWpanTschNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  // Maximum payload size is: max psdu - frame control - seqno - addressing - security - fcs
  //                        = 128      - 2             - 1     - (2+2+0+0)  - 0        - 2
  //                        = 118
  // assuming no security and addressing with only 16 bit addresses without pan id compression.
  return 118;
}

bool
LrWpanTschNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy != 0 && m_linkUp;
}

void
LrWpanTschNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
  m_linkChanges.ConnectWithoutContext (callback);
}

bool
LrWpanTschNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
LrWpanTschNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac16Address ("ff:ff");
}

bool
LrWpanTschNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
LrWpanTschNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_ABORT_MSG ("Unsupported");
  return Address ();
}

Address
LrWpanTschNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this);
  /* Implementation based on RFC 4944 Section 9.
   * An IPv6 packet with a multicast destination address (DST),
   * consisting of the sixteen octets DST[1] through DST[16], is
   * transmitted to the following 802.15.4 16-bit multicast address:
   *           0                   1
   *           0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
   *          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *          |1 0 0|DST[15]* |   DST[16]     |
   *          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * Here, DST[15]* refers to the last 5 bits in octet DST[15], that is,
   * bits 3-7 within DST[15].  The initial 3-bit pattern of "100" follows
   * the 16-bit address format for multicast addresses (Section 12). */

  // \todo re-add this once Lr-Wpan will be able to accept these multicast addresses
  //  uint8_t buf[16];
  //  uint8_t buf2[2];
  //
  //  addr.GetBytes(buf);
  //
  //  buf2[0] = 0x80 | (buf[14] & 0x1F);
  //  buf2[1] = buf[15];
  //
  //  Mac16Address newaddr = Mac16Address();
  //  newaddr.CopyFrom(buf2);
  //  return newaddr;

  return Mac16Address ("ff:ff");
}

bool
LrWpanTschNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
LrWpanTschNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
LrWpanTschNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  // This method basically assumes an 802.3-compliant device, but a raw
  // 802.15.4 device does not have an ethertype, and requires specific
  // McpsDataRequest parameters.
  // For further study:  how to support these methods somehow, such as
  // inventing a fake ethertype and packet tag for McpsDataRequest
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  if (packet->GetSize () > GetMtu ())
    {
      NS_LOG_ERROR ("Fragmentation is needed for this packet, drop the packet ");
      return false;
    }

  assert(m_isTsch>=0);
  if ( m_isTsch) {
	  TschMcpsDataRequestParams m_mcpsDataRequestParams;
	  LrWpanFrameControlOptions frmcontrol;
	  m_mcpsDataRequestParams.m_frameControlOptions = frmcontrol;
	  m_mcpsDataRequestParams.m_dstAddr = Mac16Address::ConvertFrom (dest);
	  m_mcpsDataRequestParams.m_dstAddrMode = SHORT_ADDR;
	  m_mcpsDataRequestParams.m_srcAddrMode = NO_PANID_ADDR;

	  m_mcpsDataRequestParams.m_ACK_TX = (Mac16Address::ConvertFrom (dest) == Mac16Address("ff:ff")) ? false : m_useAcks;
	  m_mcpsDataRequestParams.m_msduHandle = 0;

	  m_mcpsDataRequestParams.m_dstPanId = m_mac->GetPanId ();
	  m_mac->McpsDataRequest (m_mcpsDataRequestParams, packet);
  } else {
	  McpsDataRequestParams m_mcpsDataRequestParams;
	  m_mcpsDataRequestParams.m_dstAddr = Mac16Address::ConvertFrom (dest);
	  m_mcpsDataRequestParams.m_dstAddrMode = SHORT_ADDR;
	  m_mcpsDataRequestParams.m_dstPanId = m_omac->GetPanId ();
	  m_mcpsDataRequestParams.m_srcAddrMode = SHORT_ADDR;

	  m_mcpsDataRequestParams.m_txOptions = m_useAcks ? TX_OPTION_ACK : 0;
	  m_mcpsDataRequestParams.m_msduHandle = 0;

	  m_omac->McpsDataRequest (m_mcpsDataRequestParams, packet);
  }
  return true;
}

bool
LrWpanTschNetDevice::Send (Ptr<Packet> packet, const Address& dest, bool use_ack, uint16_t protocolNumber)
{
  // This method basically assumes an 802.3-compliant device, but a raw
  // 802.15.4 device does not have an ethertype, and requires specific
  // McpsDataRequest parameters.
  // For further study:  how to support these methods somehow, such as
  // inventing a fake ethertype and packet tag for McpsDataRequest
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  if (packet->GetSize () > GetMtu ())
    {
      NS_LOG_ERROR ("Fragmentation is needed for this packet, drop the packet ");
      return false;
    }

  assert(m_isTsch>=0);
  if ( m_isTsch) {
	  TschMcpsDataRequestParams m_mcpsDataRequestParams;
	  LrWpanFrameControlOptions frmcontrol;
	  m_mcpsDataRequestParams.m_frameControlOptions = frmcontrol;
	  m_mcpsDataRequestParams.m_dstAddr = Mac16Address::ConvertFrom (dest);
	  m_mcpsDataRequestParams.m_dstAddrMode = SHORT_ADDR;
	  m_mcpsDataRequestParams.m_srcAddrMode = NO_PANID_ADDR;
	  m_mcpsDataRequestParams.m_ACK_TX = use_ack;
	  m_mcpsDataRequestParams.m_msduHandle = 0;

	  m_mcpsDataRequestParams.m_dstPanId = m_mac->GetPanId ();
	  m_mac->McpsDataRequest (m_mcpsDataRequestParams, packet);
  } else {
	  McpsDataRequestParams m_mcpsDataRequestParams;
	  m_mcpsDataRequestParams.m_dstAddr = Mac16Address::ConvertFrom (dest);
	  m_mcpsDataRequestParams.m_dstAddrMode = SHORT_ADDR;
	  m_mcpsDataRequestParams.m_dstPanId = m_omac->GetPanId ();
	  m_mcpsDataRequestParams.m_srcAddrMode = SHORT_ADDR;

	  m_mcpsDataRequestParams.m_txOptions = use_ack ? TX_OPTION_ACK : 0;
	  m_mcpsDataRequestParams.m_msduHandle = 0;

	  m_omac->McpsDataRequest (m_mcpsDataRequestParams, packet);
  }
  return true;
}
bool
LrWpanTschNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_ABORT_MSG ("Unsupported");
  // TODO: To support SendFrom, the MACs McpsDataRequest has to use the provided source address, instead of to local one.
  return false;
}

Ptr<Node>
LrWpanTschNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

void
LrWpanTschNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  m_node = node;
  CompleteConfig ();
}

bool
LrWpanTschNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

void
LrWpanTschNetDevice::SetReceiveCallback (ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_receiveCallback = cb;
}

void
LrWpanTschNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  // This method basically assumes an 802.3-compliant device, but a raw
  // 802.15.4 device does not have an ethertype, and requires specific
  // McpsDataIndication parameters.
  // For further study:  how to support these methods somehow, such as
  // inventing a fake ethertype and packet tag for McpsDataRequest
  NS_LOG_WARN ("Unsupported; use LrWpan MAC APIs instead");
}

void
LrWpanTschNetDevice::McpsDataIndication (McpsDataIndicationParams params, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this);
  // TODO: Use the PromiscReceiveCallback if the MAC is in promiscuous mode.
  m_receiveCallback (this, pkt, 0, params.m_srcAddr);
}

bool
LrWpanTschNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return false;
}

int64_t
LrWpanTschNetDevice::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (stream);
  int64_t streamIndex = stream;
  streamIndex += m_csmaca->AssignStreams (stream);
  streamIndex += m_phy->AssignStreams (stream);
  NS_LOG_DEBUG ("Number of assigned RV streams:  " << (streamIndex - stream));
  return (streamIndex - stream);
}

/***** TSCH ************/
void
LrWpanTschNetDevice::SlotframeConfirm (MlmeSetSlotframeConfirmParams params)
{
  NS_LOG_FUNCTION (this);
}

void
LrWpanTschNetDevice::LinkConfirm (MlmeSetLinkConfirmParams params)
{
  NS_LOG_FUNCTION (this);
}

void
LrWpanTschNetDevice::ModeConfirm (MlmeTschModeConfirmParams params)
{
  NS_LOG_FUNCTION (this);
}

void
LrWpanTschNetDevice::SetTschMode (bool enable)
{
  NS_LOG_FUNCTION (this);
  if (m_isTsch==true && enable) return;
  if (m_isTsch==false && !enable) return;
  if (m_isTsch!=true && enable) {
	  m_phy->SetPdDataIndicationCallback (MakeCallback (&LrWpanTschMac::PdDataIndication, m_mac));
	  m_phy->SetPdDataConfirmCallback (MakeCallback (&LrWpanTschMac::PdDataConfirm, m_mac));
	  m_phy->SetPlmeEdConfirmCallback (MakeCallback (&LrWpanTschMac::PlmeEdConfirm, m_mac));
	  m_phy->SetPlmeGetAttributeConfirmCallback (MakeCallback (&LrWpanTschMac::PlmeGetAttributeConfirm, m_mac));
	  m_phy->SetPlmeSetTRXStateConfirmCallback (MakeCallback (&LrWpanTschMac::PlmeSetTRXStateConfirm, m_mac));
	  m_phy->SetPlmeSetAttributeConfirmCallback (MakeCallback (&LrWpanTschMac::PlmeSetAttributeConfirm, m_mac));

	  m_phy->SetPlmeCcaConfirmCallback (MakeCallback (&LrWpanTschMac::PlmeCcaConfirm, m_mac));

	  MlmeTschModeRequestParams modeRequest;
	  modeRequest.TSCHMode = MlmeTschMode_ON;
	  m_mac->MlmeTschModeRequest(modeRequest);
	  m_isTsch=true;
  }
  if (m_isTsch!=false && !enable) {
	  MlmeTschModeRequestParams modeRequest;
	  modeRequest.TSCHMode = MlmeTschMode_OFF;
	  m_mac->MlmeTschModeRequest(modeRequest);

	  m_phy->SetPdDataIndicationCallback (MakeCallback (&LrWpanMac::PdDataIndication, m_omac));
	  m_phy->SetPdDataConfirmCallback (MakeCallback (&LrWpanMac::PdDataConfirm, m_omac));
	  m_phy->SetPlmeEdConfirmCallback (MakeCallback (&LrWpanMac::PlmeEdConfirm, m_omac));
	  m_phy->SetPlmeGetAttributeConfirmCallback (MakeCallback (&LrWpanMac::PlmeGetAttributeConfirm, m_omac));
	  m_phy->SetPlmeSetTRXStateConfirmCallback (MakeCallback (&LrWpanMac::PlmeSetTRXStateConfirm, m_omac));
	  m_phy->SetPlmeSetAttributeConfirmCallback (MakeCallback (&LrWpanMac::PlmeSetAttributeConfirm, m_omac));

	  m_phy->SetPlmeCcaConfirmCallback (MakeCallback (&LrWpanCsmaCa::PlmeCcaConfirm, m_csmaca));

	  m_isTsch=false;
  }
}
} // namespace ns3
