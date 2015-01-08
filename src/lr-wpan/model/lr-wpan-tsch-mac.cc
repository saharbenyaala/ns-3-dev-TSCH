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
 *  kwong yin <kwong-sang.yin@boeing.com>
 *  Tom Henderson <thomas.r.henderson@boeing.com>
 *  Sascha Alexander Jopen <jopen@cs.uni-bonn.de>
 *  Erwan Livolant <erwan.livolant@inria.fr>
 *  Luis Pacheco <luisbelem@gmail.com>
 *  Peishuo Li <pressthunder@gmail.com>
 *  Peter Kourzanov <peter.kourzanov@gmail.com>
 */

#include "lr-wpan-tsch-mac.h"
#include "lr-wpan-csmaca.h"
#include "lr-wpan-mac-trailer.h"
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/uinteger.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/random-variable-stream.h>
#include <ns3/double.h>

NS_LOG_COMPONENT_DEFINE ("LrWpanTschMac");

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT                                   \
  std::clog << "[address " << m_shortAddress << "] ";

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LrWpanTschMac);

const uint32_t LrWpanTschMac::aMinMPDUOverhead = 9; // Table 85

TypeId
LrWpanTschMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LrWpanTschMac")
    .SetParent<Object> ()
    .AddTraceSource ("MacTxEnqueue",
                     "Trace source indicating a packet has was enqueued in the transaction queue",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macTxEnqueueTrace))
    .AddTraceSource ("MacTxDequeue",
                     "Trace source indicating a packet has was dequeued from the transaction queue",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macTxDequeueTrace))
    .AddTraceSource ("MacTx",
                     "Trace source indicating a packet has arrived for transmission by this device",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macTxTrace))
    .AddTraceSource ("MacMaxRetries",
                     "Trace source indicating the maximum number of retries has been reached",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macMaxRetries))
    .AddTraceSource ("MacTxOk",
                     "Trace source indicating a packet has been successfully sent",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macTxOkTrace))
    .AddTraceSource ("MacTxDrop",
                     "Trace source indicating a packet has been dropped during transmission",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macTxDropTrace))
    .AddTraceSource ("MacPromiscRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macPromiscRxTrace))
    .AddTraceSource ("MacRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macRxTrace))
    .AddTraceSource ("MacRxDrop",
                     "Trace source indicating a packet was received, but dropped before being forwarded up the stack",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macRxDropTrace))
    .AddTraceSource ("Sniffer",
                     "Trace source simulating a non-promiscuous packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_snifferTrace))
    .AddTraceSource ("PromiscSniffer",
                     "Trace source simulating a promiscuous packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_promiscSnifferTrace))
    .AddTraceSource ("MacState",
                     "The state of LrWpan Mac",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macStateLogger))
    .AddTraceSource ("MacSentPkt",
                     "Trace source reporting some information about the sent packet",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_sentPktTrace))

      //Timeslot type tracing
    .AddTraceSource ("MacEmptyBuffer",
                     "Device has no packet at its buffer",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macRxEmptyBufferTrace))
    .AddTraceSource ("MacRxDataTxAck",
                     "Device receives a data packet and sends an ACK",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macRxDataTxAckTrace))
    .AddTraceSource ("MacTxData",
                     "Device sends a data packet",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macTxDataTrace))
    .AddTraceSource ("MacRxData",
                     "Device receives a data packet",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macRxDataTrace))
    .AddTraceSource ("MacTxDataRxAck",
                     "Device sends a data packet and receives an ACK",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macTxDataRxAckTrace))
    .AddTraceSource ("MacSleep",
                     "The timeslot is not assigned for the device",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macSleepTrace))
    .AddTraceSource ("MacIdle",
                     "Device listens for a packet but does not receive one",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macIdleTrace))
    .AddTraceSource ("MacChannelBusy",
                     "Device performs a CCA and the channel is busy",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macChannelBusyTrace))
    .AddTraceSource ("MacWaitAck",
                     "Device sends a data packet, listens for an ACK but does not receive one",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macWaitAckTrace))
    .AddTraceSource ("MacLinkInformation",
                     "Received power and bias power, Channel, Rx and Tx Node ID.",
                     MakeTraceSourceAccessor (&LrWpanTschMac::m_macLinkInformation))
      ;
  return tid;
}

LrWpanTschMac::LrWpanTschMac ()
{
  // First set the state to a known value, call ChangeMacState to fire trace source.
  m_lrWpanMacState = TSCH_MAC_IDLE;
  ChangeMacState (TSCH_MAC_IDLE);
  m_lrWpanMacStatePending = TSCH_MAC_IDLE;

  m_macPanId = 0;
  m_associationStatus = ASSOCIATED;
  m_selfExt = Mac64Address::Allocate ();
  m_shortAddress = Mac16Address::Allocate();
  m_macPromiscuousMode = false;
  m_macMaxFrameRetries = 5;
  m_txPkt = 0;
  m_txLinkSequence = 0;

  Ptr<UniformRandomVariable> uniformVar = CreateObject<UniformRandomVariable> ();
  uniformVar->SetAttribute ("Min", DoubleValue (0.0));
  uniformVar->SetAttribute ("Max", DoubleValue (255.0));
  m_macDsn = SequenceNumber8 (uniformVar->GetValue ());

  m_macCCAEnabled = true;
  m_macHoppingEnabled = true;
  m_sharedLink = false;
  m_emptySlot = true;
  m_newSlot = true;
  m_random = CreateObject<UniformRandomVariable> ();

  ResetMacTschPibAttributes();
  ResetMacTimeslotTemplate();
  SetDefaultHoppingSequence(16);
}

LrWpanTschMac::~LrWpanTschMac ()
{
}

void
LrWpanTschMac::DoInitialize ()
{
  m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TRX_OFF);
  Object::DoInitialize ();
}

void
LrWpanTschMac::DoDispose ()
{
  /*if (m_csmaCa != 0)retransmi
    {
      m_csmaCa->Dispose ();
      m_csmaCa = 0;
    }*/
  m_txPkt = 0;
  m_txLinkSequence = 0;

  for (uint32_t i = 0; i < m_txQueueAllLink.size (); i++)
    {
      for (uint32_t j = 0; j < m_txQueueAllLink[i]->txQueuePerLink.size(); j++)
        {
          m_txQueueAllLink[i]->txQueuePerLink[j]->txQPkt = 0;
          delete m_txQueueAllLink[i]->txQueuePerLink[j];
        }
      m_txQueueAllLink[i]->txQueuePerLink.clear();
      delete m_txQueueAllLink[i];
    }
  m_txQueueAllLink.clear ();

  m_phy = 0;
  m_mcpsDataIndicationCallback = MakeNullCallback< void, McpsDataIndicationParams, Ptr<Packet> > ();
  m_mcpsDataConfirmCallback = MakeNullCallback< void, McpsDataConfirmParams > ();

  m_mlmeSetSlotframeConfirmCallback = MakeNullCallback< void, MlmeSetSlotframeConfirmParams > ();
  m_mlmeTschModeConfirmCallback = MakeNullCallback< void, MlmeTschModeConfirmParams > ();
  m_mlmeSetLinkConfirmCallback = MakeNullCallback< void, MlmeSetLinkConfirmParams > ();

  Object::DoDispose ();
}

void
LrWpanTschMac::SetShortAddress (Mac16Address address)
{
  //NS_LOG_FUNCTION (this << address);
  m_shortAddress = address;
}

void
LrWpanTschMac::SetExtendedAddress (Mac64Address address)
{
  //NS_LOG_FUNCTION (this << address);
  m_selfExt = address;
}

Mac16Address
LrWpanTschMac::GetShortAddress () const
{
  NS_LOG_FUNCTION (this);
  return m_shortAddress;
}

Mac64Address
LrWpanTschMac::GetExtendedAddress () const
{
  NS_LOG_FUNCTION (this);
  return m_selfExt;
}

void
LrWpanTschMac::McpsDataRequest (TschMcpsDataRequestParams params, Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  McpsDataConfirmParams confirmParams;
  confirmParams.m_msduHandle = params.m_msduHandle;

  // TODO: We need a drop trace for the case that the packet is too large or the request parameters are maleformed.
  //       The current tx drop trace is not suitable, because packets dropped using this trace carry the mac header
  //       and footer, while packets being dropped here do not have them.

  LrWpanMacHeader macHdr (LrWpanMacHeader::LRWPAN_MAC_DATA, m_macDsn.GetValue ());
  m_macDsn++;

  if (p->GetSize () > LrWpanPhy::aMaxPhyPacketSize - aMinMPDUOverhead)
    {
      // Note, this is just testing maximum theoretical frame size per the spec
      // The frame could still be too large once headers are put on
      // in which case the phy will reject it instead
      NS_LOG_ERROR (this << " packet too big: " << p->GetSize ());
      confirmParams.m_status = IEEE_802_15_4_FRAME_TOO_LONG;
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          m_mcpsDataConfirmCallback (confirmParams);
        }
      return;
    }


  macHdr.SetFrameVer(2);
  if ((params.m_srcAddrMode == NO_PANID_ADDR)
      && (params.m_dstAddrMode == NO_PANID_ADDR))
    {
      NS_LOG_ERROR (this << " Can not send packet with no Address field" );
      confirmParams.m_status = IEEE_802_15_4_INVALID_ADDRESS;
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          m_mcpsDataConfirmCallback (confirmParams);
        }
      return;
    }


  macHdr.SetNoPanIdComp ();

  switch (params.m_srcAddrMode)
    {
    case NO_PANID_ADDR:
      macHdr.SetSrcAddrMode (params.m_srcAddrMode);
      break;
    case ADDR_MODE_RESERVED:
      macHdr.SetSrcAddrMode (params.m_srcAddrMode);
      break;
    case SHORT_ADDR:
      macHdr.SetSrcAddrMode (params.m_srcAddrMode);
      macHdr.SetSrcAddrFields (GetPanId (), GetShortAddress ());
      break;
    case EXT_ADDR:
      macHdr.SetSrcAddrMode (params.m_srcAddrMode);
      macHdr.SetSrcAddrFields (GetPanId (), GetExtendedAddress ());
      break;
    default:
      NS_LOG_ERROR (this << " Can not send packet with incorrect Source Address mode = " << params.m_srcAddrMode);
      confirmParams.m_status = IEEE_802_15_4_INVALID_ADDRESS;
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          m_mcpsDataConfirmCallback (confirmParams);
        }
      return;
    }

  if (params.m_SecurityLevel == 0)
    {
      macHdr.SetSecDisable ();
    }
  else
    {
      //TODO
    }

  if (params.m_frameControlOptions.m_PanIdSupressed)
    {
      macHdr.SetPanIdComp();
    }
  else
    {
      macHdr.SetNoPanIdComp();
    }

  if (params.m_frameControlOptions.IesIncluded)
    {
      macHdr.SetIEField();
      //TODO: Insert IEs
    }
  else
    {
      macHdr.SetNoIEField();
    }

  if (params.m_frameControlOptions.SeqNSupressed)
    {
      macHdr.SetSeqNumSup();
    }
  else
    {
      macHdr.SetNoSeqNumSup();
      macHdr.SetSeqNum(m_macDsn.GetValue());
    }

  if (params.m_sendMultipurpose)
    {
      //TODO
    }
  else if (params.m_frameControlOptions.SeqNSupressed || params.m_frameControlOptions.IesIncluded)
    {
      //macHdr.SetFrameVer(2);
    }
  else
    {
      //macHdr.SetFrameVer(1);
    }

  if (params.m_ACK_TX)
    {
      macHdr.SetAckReq ();
    }
  else
    {
      macHdr.SetNoAckReq ();
    }

  if (params.m_GTSTX)
    {
      //TODO
    }
  else if (params.m_IndirectTx)
    {
      //TODO: indirect tx, overrrided by gts
    }

  if ((macHdr.GetFrameVer() == 1 || macHdr.GetFrameVer() == 0) && params.m_dstAddrMode == 0)
    {
      //TODO: frame directed to PAN coordinator, with the the pan id as src pan id
    }

  if ((macHdr.GetFrameVer() == 1 || macHdr.GetFrameVer() == 0) && params.m_srcAddrMode == 0)
    {
      //TODO: frame originated from PAN coordinator, with the the pan id as dst pan id
    }

  if (macHdr.GetFrameVer() == 2)
    {
      //TODO: can be broadcast
    }

  macHdr.SetDstAddrMode (params.m_dstAddrMode);
  // TODO: Add field for EXT_ADDR destination address (and use it here).
  macHdr.SetDstAddrFields (params.m_dstPanId, params.m_dstAddr);
  macHdr.SetSecDisable ();

  p->AddHeader (macHdr);

  LrWpanMacTrailer macTrailer;
  // Calculate FCS if the global attribute ChecksumEnable is set.
  if (Node::ChecksumEnabled ())
    {
      macTrailer.EnableFcs (true);
      macTrailer.SetFcs (p);
    }
  p->AddTrailer (macTrailer);

  m_macTxEnqueueTrace (p);

  TxQueueRequestElement *txQElement = new TxQueueRequestElement;
  txQElement->txQMsduHandle = params.m_msduHandle;
  txQElement->txQPkt = p;
  txQElement->txRequestNB = 0;
  txQElement->txRequestCW = 0;

  Mac16Address dstAddr = macHdr.GetShortDstAddr ();

  bool flag_findLinkQueue = false;
  if (m_txQueueAllLink.size () == 0){
    flag_findLinkQueue = false;
    }
  else{
      for (uint32_t i = 0; i < m_txQueueAllLink.size (); i++) {
          if (m_txQueueAllLink[i]->txDstAddr == dstAddr){
              m_txQueueAllLink[i]->txQueuePerLink.push_back(txQElement);
              NS_LOG_DEBUG("Enqueuing packet with SeqNum = " << (int)macHdr.GetSeqNum()
                           << " in existed link queue with link sequence = " << i);
              flag_findLinkQueue = true;
              break;
            }
          }
       }
  if (!flag_findLinkQueue)
     SetTxLinkQueue (txQElement, dstAddr, macHdr.GetSeqNum());

}

void
LrWpanTschMac::SetTxLinkQueue (TxQueueRequestElement * newRequestElement, Mac16Address newDstAddr, uint8_t newSeqNum)
{

  NS_LOG_FUNCTION (this);

  TxQueueLinkElement *txQueueLinkElement = new TxQueueLinkElement;

  txQueueLinkElement->txQueuePerLink.push_back (newRequestElement);

  NS_LOG_DEBUG("Enqueuing packet with SeqNum = " << (int)newSeqNum
               << " in queue with link sequence = " << m_txQueueAllLink.size());

  txQueueLinkElement->txDstAddr = newDstAddr;
  txQueueLinkElement->txLinkBE = m_macTschPIBAttributes.macMinBE;

  m_txQueueAllLink.push_back (txQueueLinkElement);
}

/*void
LrWpanTschMac::SetCsmaCa (Ptr<LrWpanCsmaCa> csmaCa)
{
  m_csmaCa = csmaCa;
}*/

void
LrWpanTschMac::SetPhy (Ptr<LrWpanPhy> phy)
{
  m_phy = phy;
}

Ptr<LrWpanPhy>
LrWpanTschMac::GetPhy (void)
{
  return m_phy;
}

void
LrWpanTschMac::SetMcpsDataIndicationCallback (McpsDataIndicationCallback c)
{
  m_mcpsDataIndicationCallback = c;
}

void
LrWpanTschMac::SetMcpsDataConfirmCallback (McpsDataConfirmCallback c)
{
  m_mcpsDataConfirmCallback = c;
}

void 
LrWpanTschMac::SetMlmeSetSlotframeConfirmCallback (MlmeSetSlotframeConfirmCallback c)
{
  m_mlmeSetSlotframeConfirmCallback = c;
}
void 
LrWpanTschMac::SetMlmeTschModeConfirmCallback (MlmeTschModeConfirmCallback c)
{
  m_mlmeTschModeConfirmCallback = c;
}
void 
LrWpanTschMac::SetMlmeSetLinkConfirmCallback (MlmeSetLinkConfirmCallback c)
{
  m_mlmeSetLinkConfirmCallback = c;
}

/*
void 
SetMlmeKeepAliveConfirmCallback (MlmeKeepAliveConfirmCallback c)
{
  m_mlmeKeepAliveConfirmCallback = c;
}*/

void
LrWpanTschMac::PdDataIndication (uint32_t psduLength, Ptr<Packet> p, uint8_t lqi)
{
  NS_ASSERT (m_lrWpanMacState == TSCH_MAC_ACK_PENDING  || TSCH_MAC_ACK_PENDING_END || TSCH_MAC_RX || TSCH_PKT_WAIT_END);

  NS_LOG_FUNCTION (this << psduLength << p << (int)lqi);

  bool acceptFrame;

  // from sec 7.5.6.2 Reception and rejection, Std802.15.4-2006
  // level 1 filtering, test FCS field and reject if frame fails
  // level 2 filtering if promiscuous mode pass frame to higher layer otherwise perform level 3 filtering
  // level 3 filtering accept frame
  // if Frame type and version is not reserved, and
  // if there is a dstPanId then dstPanId=m_macPanId or broadcastPanI, and
  // if there is a shortDstAddr then shortDstAddr =shortMacAddr or broadcastAddr, and
  // if beacon frame then srcPanId = m_macPanId
  // if only srcAddr field in Data or Command frame,accept frame if srcPanId=m_macPanId

  Ptr<Packet> originalPkt = p->Copy (); // because we will strip headers

  m_promiscSnifferTrace (originalPkt);

  m_macPromiscRxTrace (originalPkt);
  // XXX no rejection tracing (to macRxDropTrace) being performed below

  LrWpanMacTrailer receivedMacTrailer;
  p->RemoveTrailer (receivedMacTrailer);
  if (Node::ChecksumEnabled ())
    {
      receivedMacTrailer.EnableFcs (true);
    }

  // level 1 filtering
  if (!receivedMacTrailer.CheckFcs (p))
    {
      m_macRxDropTrace (originalPkt);
      NS_LOG_DEBUG("FCS check fail");
    }
  else
    {
      LrWpanMacHeader receivedMacHdr;
      p->RemoveHeader (receivedMacHdr);

      McpsDataIndicationParams params;        
      if (receivedMacHdr.IsSeqNumSup())
          {
            params.m_dsn = 0;
          }
        else
          {
            params.m_dsn = receivedMacHdr.GetSeqNum ();
          }
      params.m_mpduLinkQuality = lqi;
      params.m_srcPanId = receivedMacHdr.GetSrcPanId ();
      params.m_srcAddrMode = receivedMacHdr.GetSrcAddrMode ();
      // TODO: Add field for EXT_ADDR source address.
      if (params.m_srcAddrMode == SHORT_ADDR)
        {
          params.m_srcAddr = receivedMacHdr.GetShortSrcAddr ();
        }
      params.m_dstPanId = receivedMacHdr.GetDstPanId ();
      params.m_dstAddrMode = receivedMacHdr.GetDstAddrMode ();
      // TODO: Add field for EXT_ADDR destination address.
      if (params.m_dstAddrMode == SHORT_ADDR)
        {
          params.m_dstAddr = receivedMacHdr.GetShortDstAddr ();
        }

      NS_LOG_DEBUG ("Packet from " << params.m_srcAddr << " to " << params.m_dstAddr << " with seqnum = " << (int)receivedMacHdr.GetSeqNum());

      if (m_macPromiscuousMode)
        {
          //level 2 filtering
          if (!m_mcpsDataIndicationCallback.IsNull ())
            {
              NS_LOG_DEBUG ("promiscuous mode, forwarding up");
              m_mcpsDataIndicationCallback (params, p);
            }
          else
            {
              NS_LOG_ERROR (this << " Data Indication Callback not initialised");
            }
        }
      else
        {
          //level 3 frame filtering
          acceptFrame = (receivedMacHdr.GetType () != LrWpanMacHeader::LRWPAN_MAC_RESERVED);
          if (acceptFrame)
            {
              acceptFrame = (receivedMacHdr.GetFrameVer () == 2);
            }

            if (acceptFrame && receivedMacHdr.GetFrameVer () == 2 &&
                (
                  (receivedMacHdr.GetDstAddrMode() == 0 && receivedMacHdr.GetSrcAddrMode() == 0 && receivedMacHdr.IsPanIdComp()) ||
                  (receivedMacHdr.GetDstAddrMode() > 0 && receivedMacHdr.GetSrcAddrMode() == 0 && !receivedMacHdr.IsPanIdComp()) ||
                  (receivedMacHdr.GetDstAddrMode() > 0 && receivedMacHdr.GetSrcAddrMode() > 0 && !receivedMacHdr.IsPanIdComp())
                ))
              {
                acceptFrame = receivedMacHdr.GetDstPanId () == m_macPanId
                || receivedMacHdr.GetDstPanId () == 0xffff;
              }


          if (acceptFrame
              && (receivedMacHdr.GetDstAddrMode () == 2))
            {
              acceptFrame = receivedMacHdr.GetShortDstAddr () == m_shortAddress
                || receivedMacHdr.GetShortDstAddr () == Mac16Address ("ff:ff");      // check for broadcast addrs
            }

          if (acceptFrame
              && (receivedMacHdr.GetDstAddrMode () == 3))
            {
              acceptFrame = (receivedMacHdr.GetExtDstAddr () == m_selfExt);
            }


          if (acceptFrame
              && (receivedMacHdr.GetType () == LrWpanMacHeader::LRWPAN_MAC_BEACON))
            {
              if (m_macPanId == 0xffff)
                {
                  acceptFrame = true;
                }
              else
                {
                  acceptFrame = receivedMacHdr.GetSrcPanId () == m_macPanId;NS_LOG_DEBUG (acceptFrame << "-5");
                }
            }

          if (acceptFrame)
            {
              m_macRxTrace (originalPkt);
              
              if (receivedMacHdr.IsAcknowledgment () && (m_lrWpanMacState == TSCH_MAC_ACK_PENDING || m_lrWpanMacState == TSCH_MAC_ACK_PENDING_END))
                {
                  LrWpanMacHeader macHdr;
                  m_txPkt->PeekHeader (macHdr);

                  m_macTxDataRxAckTrace(m_latestPacketSize);
                  m_setMacState.Cancel ();
                  m_setMacState = Simulator::ScheduleNow (&LrWpanTschMac::SetLrWpanMacState, this, TSCH_MAC_IDLE);
                  if (receivedMacHdr.IsSeqNumSup() || (receivedMacHdr.GetSeqNum () == macHdr.GetSeqNum ()))
                    {
                      m_macTxOkTrace (m_txPkt);
                      // If it is an ACK with the expected sequence number, finish the transmission
                      // and notify the upper layer.
                      if (!m_mcpsDataConfirmCallback.IsNull ())
                        {
                          TxQueueRequestElement *txQElement = m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.front ();
                          McpsDataConfirmParams confirmParams;
                          confirmParams.m_msduHandle = txQElement->txQMsduHandle;
                          confirmParams.m_status = IEEE_802_15_4_SUCCESS;
                          m_mcpsDataConfirmCallback (confirmParams);
                        }
                      RemoveTxQueueElement ();
                      NS_LOG_DEBUG ("ACK successfully received " << (int)receivedMacHdr.GetSeqNum ());

                      //TODO: check if it is a nack
                    }
                  else
                    {
                      NS_LOG_DEBUG ("ACK received with wrong seq num" << m_selfExt);
                      HandleTxFailure ();
                    }

                    if (m_lrWpanMacState == TSCH_MAC_ACK_PENDING_END)
                      {
                        ChangeMacState(TSCH_MAC_IDLE);
                      }
                    else
                      {
                        Simulator::ScheduleNow(&LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_IDLE);
                      }
                }
              else if (receivedMacHdr.IsData () && !m_mcpsDataIndicationCallback.IsNull  ()
                       && (m_lrWpanMacState == TSCH_MAC_RX || m_lrWpanMacState == TSCH_PKT_WAIT_END))
                {
                  // If it is a data frame, push it up the stack.
                  NS_LOG_DEBUG ("Packet successfully received from " << params.m_srcAddr);
                  m_mcpsDataIndicationCallback (params, p);
                  m_latestPacketSize = originalPkt->GetSize();
                  //TODO: check the src MAC address
                  if (receivedMacHdr.IsAckReq ())
                    {
                      NS_LOG_DEBUG("Sending ack for a data packet.");
                      Simulator::Schedule(MicroSeconds(def_MacTimeslotTemplate.m_macTsTxAckDelay),&LrWpanTschMac::SendAck,this,
                                          receivedMacHdr.GetSeqNum(),receivedMacHdr.IsSeqNumSup());
                      m_lrWpanMacStatePending = TSCH_MAC_SENDING;
                      Simulator::ScheduleNow(&LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_IDLE);
                    }
                  else
                    {
                      m_macRxDataTrace(m_latestPacketSize);
                    }

                if (m_lrWpanMacState == TSCH_PKT_WAIT_END)
                  {
                    ChangeMacState(TSCH_MAC_IDLE);
                  }
                else
                  {
                    Simulator::ScheduleNow(&LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_IDLE);
                  }
                }
              else
                {
                  //TODO: packet not expected
                  NS_LOG_DEBUG("Packet not expected pkt type = " << receivedMacHdr.GetType());
                  if (receivedMacHdr.IsData ())
                    {
                      m_macRxDataTrace(p->GetSize());
                    }
                }
            }
          else
            {
              m_macRxDropTrace (originalPkt);
              NS_LOG_DEBUG("Filter fail");
            }
        }
    }
}

void
LrWpanTschMac::SendAck (uint8_t seqno, bool seqnumsup)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_lrWpanMacState == TSCH_MAC_IDLE);

  // Generate a corresponding ACK Frame.
  LrWpanMacHeader macHdr;
  macHdr.SetType(LrWpanMacHeader::LRWPAN_MAC_ACKNOWLEDGMENT);
  macHdr.SetFrameVer (2);
  if (!seqnumsup)
    {
      macHdr.SetNoSeqNumSup();
      macHdr.SetSeqNum(seqno);
    }
  else
    {
      macHdr.SetSeqNumSup();
    }

  macHdr.SetNoPanIdComp();
  macHdr.SetDstAddrMode(0);
  macHdr.SetSrcAddrMode(0);

  macHdr.SetIEField();
  macHdr.NewAckIE(7); //TODO: timing!
  macHdr.EndNoPayloadIE();

  LrWpanMacTrailer macTrailer;
  Ptr<Packet> ackPacket = Create<Packet> (0);
  ackPacket->AddHeader (macHdr);

  // Calculate FCS if the global attribute ChecksumEnable is set.
  if (Node::ChecksumEnabled ())
    {
      macTrailer.EnableFcs (true);
      macTrailer.SetFcs (ackPacket);
    }
  ackPacket->AddTrailer (macTrailer);

  // Enqueue the ACK packet for further processing
  // when the transmitter is activated.

  m_txPkt = ackPacket;

  NS_LOG_DEBUG("Sending ack with size = " << m_txPkt->GetSize() << " " << m_txPkt->GetSerializedSize());
  // Switch transceiver to TX mode. Proceed sending the Ack on confirm.
  SetLrWpanMacState (TSCH_MAC_SENDING);
}

void
LrWpanTschMac::RemoveTxQueueElement ()
{
  NS_LOG_FUNCTION (this);

  TxQueueRequestElement *txQElement = m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.front();
  Ptr<const Packet> p = txQElement->txQPkt;
  //m_numCsmacaRetry += m_csmaCa->GetNB () + 1;

  Ptr<Packet> pkt = p->Copy ();
  LrWpanMacHeader hdr;
  pkt->RemoveHeader (hdr);
  if (hdr.GetShortDstAddr () != Mac16Address ("ff:ff"))
    {
      if (txQElement->txRequestNB == m_macMaxFrameRetries)
        {
          NS_LOG_DEBUG ("Maximum retry reached, delete one request in the queue with link position = "<< m_txLinkSequence);
          m_macMaxRetries(p);
        }
      else
        {
          //m_sentPktTrace (p, m_retransmission + 1, m_numCsmacaRetry);
          m_sentPktTrace (p, txQElement->txRequestNB + 1);
        }
    }

  txQElement->txQPkt = 0;
  delete txQElement;

  m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.pop_front ();

  if (m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.size() == 0){
      m_txQueueAllLink.erase(m_txQueueAllLink.begin() + m_txLinkSequence);
      NS_LOG_DEBUG ("Delete queue with link position = "<< m_txLinkSequence);
    }
  m_txLinkSequence = 0;
  m_txPkt = 0;
  //m_numCsmacaRetry = 0;
  m_macTxDequeueTrace (p);
}

void
LrWpanTschMac::PdDataConfirm (LrWpanPhyEnumeration status)
{
  NS_ASSERT (m_lrWpanMacState == TSCH_MAC_SENDING);

  NS_LOG_FUNCTION (this << status << m_txQueueAllLink.size ());

  LrWpanMacHeader macHdr;
  m_txPkt->PeekHeader (macHdr);

  if (status == IEEE_802_15_4_PHY_SUCCESS)
    {
      if (!macHdr.IsAcknowledgment ())
        {
          // We have just send a regular data packet, check if we have to wait
          // for an ACK.
          NS_LOG_DEBUG("Packet transmission successful");
          if (macHdr.IsAckReq ())
            {
              Simulator::Schedule (MicroSeconds(def_MacTimeslotTemplate.m_macTsRxAckDelay),&LrWpanTschMac::WaitAck,this);
              m_lrWpanMacStatePending = TSCH_MAC_ACK_PENDING;
            }
          else
            {
              m_macTxOkTrace (m_txPkt);
              m_macTxDataTrace(m_latestPacketSize);
              // remove the copy of the packet that was just sent
              if (!m_mcpsDataConfirmCallback.IsNull ())
                {
                  McpsDataConfirmParams confirmParams;
                  NS_ASSERT_MSG (m_txQueueAllLink.size () > 0, "TxQsize = 0");
                  TxQueueRequestElement *txQElement = m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.front ();
                  confirmParams.m_msduHandle = txQElement->txQMsduHandle;
                  confirmParams.m_status = IEEE_802_15_4_SUCCESS;
                  m_mcpsDataConfirmCallback (confirmParams);
                }
              RemoveTxQueueElement ();
            }
        }
      else
        {
          NS_LOG_DEBUG("ACK transmission sussesfull");
          m_macRxDataTxAckTrace(m_latestPacketSize);
          // We have send an ACK. Clear the packet buffer.
          m_txPkt = 0;
        }
    }
  else if (status == IEEE_802_15_4_PHY_UNSPECIFIED)
    {

      if (!macHdr.IsAcknowledgment ())
        {
          NS_LOG_DEBUG("Unable to send packet");
          if ((Now().GetSeconds() - m_lastTransmission.GetSeconds()) == 0.0)
            {
              m_macRxDataTrace(m_latestPacketSize);
            }
          m_latestPacketSize = m_txPkt->GetSize();
          NS_ASSERT_MSG (m_txQueueAllLink.size () > 0, "TxQsize = 0");
          TxQueueRequestElement *txQElement = m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.front ();
          m_macTxDropTrace (txQElement->txQPkt);
          if (!m_mcpsDataConfirmCallback.IsNull ())
            {
              McpsDataConfirmParams confirmParams;
              confirmParams.m_msduHandle = txQElement->txQMsduHandle;
              confirmParams.m_status = IEEE_802_15_4_FRAME_TOO_LONG;
              m_mcpsDataConfirmCallback (confirmParams);
            }
          RemoveTxQueueElement ();
        }
      else
        {
          NS_LOG_ERROR ("Unable to send ACK");
          if ((Now().GetSeconds() - m_lastTransmission.GetSeconds()) == 0.0)
            {
              m_macRxDataTrace(m_latestPacketSize);
            }
          else
            {
              m_macRxDataTxAckTrace(m_latestPacketSize);
            }
        }
    }
  else
    {
      // Something went really wrong. The PHY is not in the correct state for
      // data transmission.
      NS_FATAL_ERROR ("Transmission attempt failed with PHY status " << status);
    }

  m_setMacState.Cancel ();
  m_setMacState = Simulator::ScheduleNow (&LrWpanTschMac::SetLrWpanMacState, this, TSCH_MAC_IDLE);
}

void
LrWpanTschMac::PlmeCcaConfirm (LrWpanPhyEnumeration status)
{
  NS_LOG_FUNCTION (this << status);
  // Direct this call through the csmaCa object
  //m_csmaCa->PlmeCcaConfirm (status);

  if (status != IEEE_802_15_4_PHY_IDLE)
    {
      NS_LOG_DEBUG("CCA failure");
      m_macChannelBusyTrace(0);
      SetLrWpanMacState (TSCH_CHANNEL_ACCESS_FAILURE);
    }
  else
    {
      NS_LOG_DEBUG("CCA successfull");
      SetLrWpanMacState (TSCH_CHANNEL_IDLE);
    }
}

void
LrWpanTschMac::PlmeEdConfirm (LrWpanPhyEnumeration status, uint8_t energyLevel)
{
  NS_LOG_FUNCTION (this << status << energyLevel);

}

void
LrWpanTschMac::PlmeGetAttributeConfirm (LrWpanPhyEnumeration status,
                                    LrWpanPibAttributeIdentifier id,
                                    LrWpanPhyPibAttributes* attribute)
{
  NS_LOG_FUNCTION (this << status << id << attribute);
}

void
LrWpanTschMac::PlmeSetTRXStateConfirm (LrWpanPhyEnumeration status)
{
  NS_LOG_FUNCTION (this << status);

  if (status == IEEE_802_15_4_PHY_FORCE_TRX_OFF)
      m_setMacState.Cancel ();

  else if (m_lrWpanMacState == TSCH_MAC_SENDING && (status == IEEE_802_15_4_PHY_TX_ON || status == IEEE_802_15_4_PHY_SUCCESS
                                               || status == IEEE_802_15_4_PHY_TRX_SWITCHING || status == IEEE_802_15_4_PHY_TRX_START))
    {
      if (status == IEEE_802_15_4_PHY_TX_ON || status == IEEE_802_15_4_PHY_SUCCESS)
      {
        NS_ASSERT (m_txPkt);

        // Start sending if we are in state SENDING and the PHY transmitter was enabled.
        m_promiscSnifferTrace (m_txPkt);
        m_snifferTrace (m_txPkt);
        m_macTxTrace (m_txPkt);
        m_lastTransmission = Now();

        LrWpanMacHeader macHdr;
        m_txPkt->PeekHeader(macHdr);
        if (macHdr.IsData())
          {
            m_latestPacketSize = m_txPkt->GetSize();
          }
        m_phy->PdDataRequest (m_txPkt->GetSize (), m_txPkt);
      }
    }
  else if (m_lrWpanMacState == TSCH_MAC_CCA && (status == IEEE_802_15_4_PHY_RX_ON || status == IEEE_802_15_4_PHY_SUCCESS))
    {
      // Start the CSMA algorithm as soon as the receiver is enabled.
      m_phy->PlmeCcaRequest();
    }
  else if (m_lrWpanMacState == TSCH_MAC_RX && (status == IEEE_802_15_4_PHY_RX_ON || status == IEEE_802_15_4_PHY_SUCCESS))
    {
      NS_LOG_DEBUG("Wait for a packet for " << def_MacTimeslotTemplate.m_macTsRxWait);
      Simulator::Schedule(MicroSeconds(def_MacTimeslotTemplate.m_macTsRxWait),&LrWpanTschMac::RxWaitDone,this);
    }
  else if (m_lrWpanMacState == TSCH_MAC_IDLE)
    {
      NS_ASSERT (status == IEEE_802_15_4_PHY_RX_ON || status == IEEE_802_15_4_PHY_SUCCESS || status == IEEE_802_15_4_PHY_TRX_OFF
                 || status == IEEE_802_15_4_PHY_TRX_SWITCHING || status == IEEE_802_15_4_PHY_TRX_START);
      // Do nothing special when going idle.
    }
  else if (m_lrWpanMacState == TSCH_PKT_WAIT_END)
    {
      if (m_macWaitDone == Now())
        {
          //didnt receive any packet
          m_macIdleTrace(0);
          ChangeMacState (TSCH_MAC_IDLE);
        }
      NS_ASSERT (status == IEEE_802_15_4_PHY_TRX_OFF || status == IEEE_802_15_4_PHY_SUCCESS);
      NS_LOG_DEBUG("End of waiting for a packet at " << Now().GetSeconds());
    }
  else if (m_lrWpanMacState == TSCH_MAC_ACK_PENDING_END)
    {
      if (m_macWaitDone == Now())
        {
          //didnt receive any packet
          m_macWaitAckTrace(m_latestPacketSize);
          NS_LOG_DEBUG("No ack received.");
          HandleTxFailure ();
          ChangeMacState (TSCH_MAC_IDLE);
        }

      NS_ASSERT (status == IEEE_802_15_4_PHY_TRX_OFF || status == IEEE_802_15_4_PHY_SUCCESS);
      NS_LOG_DEBUG("End of waiting for an ack at " << Now().GetSeconds());
    }
  else if (m_lrWpanMacState == TSCH_MAC_ACK_PENDING)
    {
      NS_ASSERT (status == IEEE_802_15_4_PHY_RX_ON || status == IEEE_802_15_4_PHY_SUCCESS);
    }
  else
    {
      // TODO: What to do when we receive an error?
      // If we want to transmit a packet, but switching the transceiver on results
      // in an error, we have to recover somehow (and start sending again).
      NS_FATAL_ERROR ("Error changing transceiver state");
    }
}

void
LrWpanTschMac::PlmeSetAttributeConfirm (LrWpanPhyEnumeration status,
                                    LrWpanPibAttributeIdentifier id)
{
  NS_LOG_FUNCTION (this << status << id);
}

void
LrWpanTschMac::SetLrWpanMacState (LrWpanTschMacState macState)
{
  NS_LOG_FUNCTION (this << " mac state = " << macState);

  McpsDataConfirmParams confirmParams;

  if (macState == TSCH_MAC_IDLE)
    {
      ChangeMacState (TSCH_MAC_IDLE);
      if (m_lrWpanMacStatePending == TSCH_MAC_IDLE)
          m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TRX_OFF);
      else
        {
          m_lrWpanMacStatePending = TSCH_MAC_IDLE;
          if (m_newSlot)
           {
              m_newSlot = false;
              m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TRX_START);
          }
              else
            m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TRX_SWITCHING);
        }
    }
  else if (macState == TSCH_MAC_ACK_PENDING)
    {
      ChangeMacState (TSCH_MAC_ACK_PENDING);
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_RX_ON);
    }
  else if (macState == TSCH_MAC_CCA || macState == TSCH_MAC_RX)
    {
      NS_ASSERT (m_lrWpanMacState == TSCH_MAC_IDLE || m_lrWpanMacState == TSCH_MAC_ACK_PENDING);

      ChangeMacState (macState);
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_RX_ON);
    }
  else if (macState == TSCH_PKT_WAIT_END || macState == TSCH_MAC_ACK_PENDING_END)
    {
      NS_ASSERT (m_lrWpanMacState == TSCH_MAC_RX || m_lrWpanMacState == TSCH_MAC_ACK_PENDING);

      ChangeMacState (macState);
      m_macWaitDone = Now();
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TRX_OFF);
    }
  else if (m_lrWpanMacState == TSCH_MAC_CCA && macState == TSCH_CHANNEL_IDLE)
    {
      // Channel is idle, set transmitter to TX_ON
      NS_LOG_DEBUG (this << " channel idle, set TX_ON");
      ChangeMacState (TSCH_MAC_SENDING);
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TX_ON);
    }
  else if (m_lrWpanMacState == TSCH_MAC_CCA && macState == TSCH_CHANNEL_ACCESS_FAILURE)
    {
      NS_ASSERT (m_txPkt);

      // cannot find a clear channel, drop the current packet.
      NS_LOG_DEBUG ( this << " cannot find clear channel");
      confirmParams.m_msduHandle = m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.front()->txQMsduHandle;
      confirmParams.m_status = IEEE_802_15_4_CHANNEL_ACCESS_FAILURE;
      if (!m_mcpsDataConfirmCallback.IsNull ())
        {
          m_mcpsDataConfirmCallback (confirmParams);
        }
      // remove the copy of the packet that was just sent
      ChangeMacState (TSCH_MAC_IDLE);
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TRX_OFF);


    }
  else if (m_lrWpanMacState == TSCH_MAC_IDLE && macState == TSCH_MAC_SENDING)
    {
      // sending without cca
      ChangeMacState (TSCH_MAC_SENDING);
      m_phy->PlmeSetTRXStateRequest (IEEE_802_15_4_PHY_TX_ON);
    }

}

LrWpanAssociationStatus
LrWpanTschMac::GetAssociationStatus (void) const
{
  return m_associationStatus;
}

void
LrWpanTschMac::SetAssociationStatus (LrWpanAssociationStatus status)
{
  m_associationStatus = status;
}

uint16_t
LrWpanTschMac::GetPanId (void) const
{
  return m_macPanId;
}

void
LrWpanTschMac::SetPanId (uint16_t PanId)
{
  m_macPanId = PanId;
}

void
LrWpanTschMac::ChangeMacState (LrWpanTschMacState newState)
{
  NS_LOG_LOGIC (this << " change lrwpan mac state from "
                     << m_lrWpanMacState << " to "
                     << newState);
  m_macStateLogger (m_lrWpanMacState, newState);
  m_lrWpanMacState = newState;
}

uint8_t
LrWpanTschMac::GetMacMaxFrameRetries (void) const
{
  return m_macMaxFrameRetries;
}

void
LrWpanTschMac::SetMacMaxFrameRetries (uint8_t retries)
{
  m_macMaxFrameRetries = retries;
}

/*
TSCH method
*/

void
LrWpanTschMac::MlmeSetSlotframeRequest (MlmeSetSlotframeRequestParams params)
{
  NS_LOG_DEBUG(this);
  MlmeSetSlotframeConfirmParams confirmParams;
  bool foundsf = false;
  switch(params.Operation) {
    case MlmeSlotframeOperation_ADD: //add
      confirmParams.slotframeHandle = params.slotframeHandle;
      if (params.slotframeHandle < 0 && params.slotframeHandle > 255) {
        confirmParams.Status = MlmeSetSlotframeConfirmStatus_INVALID_PARAMETER;
      } else {
        MacPibSlotframeAttributes entry;
        entry.slotframeHandle = params.slotframeHandle;
        entry.size = params.size;
        m_macSlotframeTable.push_back(entry);
        confirmParams.Status = MlmeSetSlotframeConfirmStatus_SUCCESS;
      }

      break;
    case MlmeSlotframeOperation_DELETE: //delete
      confirmParams.slotframeHandle = params.slotframeHandle;
      
      for (std::list<MacPibSlotframeAttributes>::iterator i = m_macSlotframeTable.begin();i != m_macSlotframeTable.end();i++)
        {
          if (i->slotframeHandle == params.slotframeHandle) {
            foundsf = true;
            confirmParams.Status = MlmeSetSlotframeConfirmStatus_SUCCESS;
            if (currentLink.active && currentLink.slotframeHandle == params.slotframeHandle)
              {
                //wait
              }
            else
              {
                //delete
              }
          }
        }

      if (!foundsf) 
        {
          confirmParams.Status = MlmeSetSlotframeConfirmStatus_SLOTFRAME_NOT_FOUND;
        }

      break;
    case MlmeSlotframeOperation_MODIFY: //modify
      confirmParams.slotframeHandle = params.slotframeHandle;
      for (std::list<MacPibSlotframeAttributes>::iterator i = m_macSlotframeTable.begin();i != m_macSlotframeTable.end();i++)
        {
          if (i->slotframeHandle == params.slotframeHandle) {
            foundsf = true;
            confirmParams.Status = MlmeSetSlotframeConfirmStatus_SUCCESS;
            i->size = params.size;
          }
        }

      if (!foundsf) 
        {
          confirmParams.Status = MlmeSetSlotframeConfirmStatus_SLOTFRAME_NOT_FOUND;
        }

      break;
    default:
      break;
  }

  if ((!m_mlmeSetSlotframeConfirmCallback.IsNull ()))
    {
      m_mlmeSetSlotframeConfirmCallback (confirmParams);
    }
  else
    {
      NS_LOG_ERROR ("m_mlmeSetSlotframeConfirmCallback not initialized");
    }
}

void
LrWpanTschMac::MlmeSetLinkRequest (MlmeSetLinkRequestParams params)
{
  NS_LOG_DEBUG(this);
  MlmeSetLinkConfirmParams confirmParams;
  confirmParams.linkHandle = params.linkHandle;
  confirmParams.slotframeHandle = params.slotframeHandle;
  MacPibLinkAttributes entry;
  bool  foundlink = false;
  //std::list<MacPibLinkAttributes>::iterator it;

  switch(params.Operation) {
    case MlmeSetLinkRequestOperation_ADD_LINK:

      /*for (it = m_macLinkTable.begin();it != m_macLinkTable.end();it++) {
        if (it->slotframeHandle == params.slotframeHandle) {

          //Check if link already exists
          if (it->macLinkHandle == params.linkHandle) {
            confirmParams.Status = MlmeSetLinkConfirmStatus_INVALID_PARAMETER; //invalid
            break;
          }
        }
      }*/
    
      entry.macLinkHandle = params.linkHandle;
      entry.macLinkOptions = params.linkOptions; //b0 = Transmit, b1 = Receive, b2 = Shared, b3= Timekeeping, b4–b7 reserved.
      entry.macLinkType = params.linkType;
      entry.slotframeHandle = params.slotframeHandle;
      entry.macNodeAddr = params.nodeAddr; //not using Mac16_Address because 0xffff means the link can be used for frames destined for the boradcast address
      entry.macTimeslot = params.Timeslot; //refer to 5.1.1.5
      entry.macChannelOffset = params.ChannelOffset; //refer to 5.1.1.5.3
      entry.macLinkFadingBias = params.linkFadingBias;
      entry.macTxID = params.TxID;
      entry.macRxID = params.RxID;

      m_macLinkTable.push_back(entry);
      confirmParams.Status = MlmeSetLinkConfirmStatus_SUCCESS;

      break;
    case MlmeSetLinkRequestOperation_DELETE_LINK:    
      for (std::list<MacPibLinkAttributes>::iterator i = m_macLinkTable.begin();i != m_macLinkTable.end();i++)
        {
          if (i->slotframeHandle == params.slotframeHandle && i->macLinkHandle == params.linkHandle) {
            foundlink = true;
            confirmParams.Status = MlmeSetLinkConfirmStatus_SUCCESS;
            if (currentLink.active && currentLink.slotframeHandle == params.slotframeHandle && currentLink.linkHandle == params.linkHandle)
              {
                m_waitingLink = true;
                m_waitingLinkParams = params;
              }
            else
              {
                m_waitingLink = false;
                m_macLinkTable.erase(i);
              }
            break;
          }
        }

      if (!foundlink) 
        {
          confirmParams.Status = MlmeSetLinkConfirmStatus_UNKNOWN_LINK;
        }
      break;
    case MlmeSetLinkRequestOperation_MODIFY_LINK:
      for (std::list<MacPibLinkAttributes>::iterator i = m_macLinkTable.begin();i != m_macLinkTable.end();i++)
        {
          if (i->slotframeHandle == params.slotframeHandle && i->macLinkHandle == params.linkHandle) {
            foundlink = true;
            confirmParams.Status = MlmeSetLinkConfirmStatus_SUCCESS;
            if (currentLink.active && currentLink.slotframeHandle == params.slotframeHandle && currentLink.linkHandle == params.linkHandle)
              {
                m_waitingLink = true;
                m_waitingLinkParams = params;
              }
            else
              {
                NS_LOG_DEBUG("TSCH modifying link");
                m_waitingLink = false;
                i->macLinkOptions = params.linkOptions; //b0 = Transmit, b1 = Receive, b2 = Shared, b3= Timekeeping, b4–b7 reserved.
                i->macLinkType = params.linkType;
                i->macNodeAddr = params.nodeAddr; //not using Mac16_Address, 0xffff means the link can be used for frames destined for the broadcast address
                i->macTimeslot = params.Timeslot; //refer to 5.1.1.5
                i->macChannelOffset = params.ChannelOffset; //refer to 5.1.1.5.3
                i->macLinkFadingBias = params.linkFadingBias;
                i->macTxID = params.TxID;
                i->macRxID = params.RxID;
              }
            break;
          }
        }

      if (!foundlink) 
        {
          confirmParams.Status = MlmeSetLinkConfirmStatus_UNKNOWN_LINK;
        }
      break;
    default:
      break;
  }

  if ((!m_mlmeSetLinkConfirmCallback.IsNull ()) && !m_waitingLink)
    {
      m_mlmeSetLinkConfirmCallback (confirmParams);
    }
  else if(!m_waitingLink)
    {
      NS_LOG_ERROR ("m_mlmeSetLinkConfirmCallback not initialized");
    }
}

void
LrWpanTschMac::MlmeTschModeRequest (MlmeTschModeRequestParams params)
{
  NS_LOG_FUNCTION(this);
  MlmeTschModeConfirmParams confirmParams;
  confirmParams.TSCHMode = params.TSCHMode;

  switch(params.TSCHMode) {
    case MlmeTschMode_ON:

      /*TODO: Check if it is synced
      if (its not synced) {
        confirmParams.Status = LrWpanMlmeTschModeConfirmStatus_NO_SYNC; //no sync
      }*/

      m_waitingLink = false;
      SetLrWpanMacState(TSCH_MAC_IDLE);
      //schedule asn incrementation
      Simulator::ScheduleNow (&LrWpanTschMac::IncAsn,this);

      confirmParams.Status = LrWpanMlmeTschModeConfirmStatus_SUCCESS; //success
      break;
    case MlmeTschMode_OFF:
      Simulator::Stop();
      confirmParams.Status = LrWpanMlmeTschModeConfirmStatus_SUCCESS;
      break;
    default:
      break;
  }

  if ((!m_mlmeTschModeConfirmCallback.IsNull ()))
    {
      m_mlmeTschModeConfirmCallback (confirmParams);
    }
  else
    {
      NS_LOG_ERROR ("m_mlmeTschModeConfirmCallback not initialized");
    }
}

void
LrWpanTschMac::IncAsn()
{
  NS_LOG_FUNCTION (this);
  m_newSlot = 1;
  m_macTschPIBAttributes.m_macASN++;
  Simulator::Schedule (MicroSeconds(def_MacTimeslotTemplate.m_macTsTimeslotLength),&LrWpanTschMac::IncAsn,this);
  currentLink.active = false;

  if (m_lrWpanMacState == TSCH_MAC_ACK_PENDING_END)
    {
      //In last timeslot an ACK was expected, the PHY received something(BUSY_RX)
      //but it didn't succeed
      NS_LOG_DEBUG("A packet was received, but not the ack");
      m_macRxDataTxAckTrace(m_latestPacketSize);
      HandleTxFailure ();
      Simulator::ScheduleNow(&LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_IDLE);
    }

  if (m_lrWpanMacState == TSCH_PKT_WAIT_END)
    {
      //In last timeslot an packet was expected but wasn't received
      NS_LOG_DEBUG("A packet was received, but not the expected one");
      m_macRxDataTrace(m_latestPacketSize);
      Simulator::ScheduleNow(&LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_IDLE);
    }

  if (m_waitingLink)
    {
      Simulator::ScheduleNow(&LrWpanTschMac::MlmeSetLinkRequest,this,m_waitingLinkParams);
    }

  for (std::list<MacPibSlotframeAttributes>::iterator it = m_macSlotframeTable.begin();it != m_macSlotframeTable.end();it++) 
    {
      Simulator::ScheduleNow(&LrWpanTschMac::ScheduleTimeslot,this,it->slotframeHandle,it->size);
    }
}

void
LrWpanTschMac::SetMacCCAEnabled(bool cca)
{
  m_macCCAEnabled = cca;
}

bool
LrWpanTschMac::GetMacCCAEnables()
{
  return m_macCCAEnabled;
}

void
LrWpanTschMac::ScheduleTimeslot(uint8_t handle, uint16_t size)
{
  uint16_t ts = m_macTschPIBAttributes.m_macASN%size;
  bool myts = false;
  m_currentReceivedPower = 0;
  NS_LOG_DEBUG("Timeslot " << m_macTschPIBAttributes.m_macASN << " ts = " << (int)ts << " Queue size = " << m_txQueueAllLink.size());

  for (std::list<MacPibLinkAttributes>::iterator it = m_macLinkTable.begin();it != m_macLinkTable.end();it++) {
    if (it->slotframeHandle == handle && it->macTimeslot == ts) {
      myts = true;
      currentLink.slotframeHandle = handle;
      currentLink.linkHandle = it->macLinkHandle;
      currentLink.active = true;

      NS_LOG_DEBUG("Link found at timeslot " << (int)ts);
      if (m_macHoppingEnabled)
        {
          //Get next channel
          m_currentChannel = def_MacChannelHopping.m_macHoppingSequenceList[
            (m_macTschPIBAttributes.m_macASN+it->macChannelOffset) % def_MacChannelHopping.m_macHoppingSequenceLength
            ];
          
          m_macTxID = it->macTxID;
          m_macRxID = it->macRxID;

          //Change channel
          NS_LOG_DEBUG("TSCH Changing to channel " << (int)m_currentChannel);
          LrWpanPhyPibAttributes *phyattr = new LrWpanPhyPibAttributes();
          phyattr->phyCurrentChannel = m_currentChannel;
          if (it->macLinkFadingBias != NULL){
              phyattr->phyLinkFadingBias = it->macLinkFadingBias[m_currentChannel-11] ;
          } else {
              phyattr->phyLinkFadingBias = 1;
	  }
	  NS_LOG_DEBUG (this << "setting for channel " << (int)m_currentChannel 
                << " fading bias: " <<
		phyattr->phyLinkFadingBias);
	  m_currentFadingBias = 10 * log10(phyattr->phyLinkFadingBias);
          Simulator::ScheduleNow (&LrWpanPhy::PlmeSetAttributeRequest,m_phy,phyCurrentChannel,phyattr);
        }

      if (it->macLinkOptions[0]) {
        //transmit
        if (it->macLinkOptions[2]) {
            m_sharedLink = true;
            NS_LOG_DEBUG("Be careful! Shared Link is Coming!");
        }
        else {
            m_sharedLink = false;
          }
        //if there is packets to be send and it is to the same addr as the link
        NS_LOG_DEBUG("Queue contained link size = " << m_txQueueAllLink.size());

        m_emptySlot = true;
        m_txPkt = FindTxPacketInEmptySlot(it->macNodeAddr);

        if (!m_emptySlot) {
              LrWpanMacHeader macHdr;
              m_txPkt->PeekHeader (macHdr);
              NS_LOG_DEBUG("Start timeslot transmiting procedure, seqnum = " << (int)macHdr.GetSeqNum());

              if(m_macCCAEnabled)
                {
                  Time time2wait = MicroSeconds(def_MacTimeslotTemplate.m_macTsCCAOffset);
                  Simulator::Schedule(time2wait, &LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_CCA);
                  m_lrWpanMacStatePending = TSCH_MAC_CCA;
                  Simulator::ScheduleNow(&LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_IDLE);
                }
              else
                {
                  Time time2wait = MicroSeconds(def_MacTimeslotTemplate.m_macTsTxOffset);
                  Simulator::Schedule (time2wait,&LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_SENDING);
                  m_lrWpanMacStatePending = TSCH_MAC_SENDING;
                  Simulator::ScheduleNow(&LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_IDLE);
                }
                break;
        } else {
          NS_LOG_DEBUG("Not sending, empty queue");
          m_macRxEmptyBufferTrace(0);
          //m_macSleepTrace (0);
         }
      } else if (it->macLinkOptions[1]) {
        //receive
        NS_LOG_DEBUG("Start timeslot receiving procedure");
        Time time2wait = MicroSeconds(def_MacTimeslotTemplate.m_macTsRxOffset);
        Simulator::Schedule (time2wait,&LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_RX);
        m_lrWpanMacStatePending = TSCH_MAC_RX;
        Simulator::ScheduleNow(&LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_IDLE);
        }
      break;
    }
  }

  //If not involved in the current timeslot turnoff the radio
  if (!myts)
    { 
      NS_LOG_DEBUG("No link in this timeslot, turning off the radio");
      Simulator::ScheduleNow (&LrWpanPhy::PlmeSetTRXStateRequest,m_phy,IEEE_802_15_4_PHY_TRX_OFF);

      m_macSleepTrace (0);
    }
}

void
LrWpanTschMac::WaitAck ()
{
  NS_LOG_FUNCTION(this);

  Simulator::ScheduleNow(&LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_ACK_PENDING);
  Simulator::Schedule(MicroSeconds(def_MacTimeslotTemplate.m_macTsAckWait),&LrWpanTschMac::AckWaitDone,this);

  uint32_t m_tempID = m_macTxID;
  m_macTxID = m_macRxID;
  m_macRxID = m_tempID;

}

void
LrWpanTschMac::AckWaitDone () {
  if (m_lrWpanMacState == TSCH_MAC_ACK_PENDING)
    {
      Simulator::ScheduleNow(&LrWpanTschMac::SetLrWpanMacState,this,TSCH_MAC_ACK_PENDING_END);
    }
  else if (m_lrWpanMacState == TSCH_MAC_IDLE)
    {
      NS_LOG_DEBUG("ACK already received");
    }
  else
    {
      NS_LOG_DEBUG("Should never be here");
    }
}

void
LrWpanTschMac::RxWaitDone () {
  if (m_lrWpanMacState == TSCH_MAC_RX)
    {
      Simulator::ScheduleNow(&LrWpanTschMac::SetLrWpanMacState,this,TSCH_PKT_WAIT_END);
    }
  else if (m_lrWpanMacState == TSCH_MAC_IDLE)
    {
      NS_LOG_DEBUG("Packet already received");
    }
  else
    {
      NS_LOG_DEBUG("Should never be here");
    }
}

void
LrWpanTschMac::ResetMacTschPibAttributes ()
{
  LrwpanMacTschPibAttributes def_MacTsch;
  
  def_MacTsch.macMinBE = 1;
  def_MacTsch.macMaxBE = 7;
  def_MacTsch.m_macDisconnectTime = 255;
  def_MacTsch.m_macJoinPriority = 1;
  def_MacTsch.m_macASN = -1;
  def_MacTsch.m_macNoHLBuffers = false;

  m_macTschPIBAttributes  = def_MacTsch;
}

void
LrWpanTschMac::SetDefaultHoppingSequence(uint16_t sequenceLength)
{
  LrWpanMacChannelHopping chtmpl;

  chtmpl.m_macHoppingSequenceID = 0;

  //TODO: get those parameters from the PHY layer
  //and correctly calculate others
  chtmpl.m_macChannelPage = 0;
  chtmpl.m_macNumberOfChannels = 16;
  chtmpl.m_macPhyConfiguration = 8;
  chtmpl.m_macExtendedBitmap.clear();
  chtmpl.m_macHoppingSequenceList.clear();
  chtmpl.m_macHoppingSequenceList.resize(sequenceLength);

  //The SHUFFLE array was previously calculated according to the standard
  //(an LFSR of 9 taps with x^9+x^5+x^0 and seed 255)
  //here only the 30 first positions are populated

  std::vector<uint8_t> SHUFFLE(sequenceLength);
  uint16_t LFSR_OUTPUT[30] = {0x0FF,0x1FE,0x0DF,0x1BE,0x05F,0x0BE,0x17C,0x1DB,0x095,0x12A,
                              0x177,0x1CD,0x0B9,0x172,0x1C7,0x0AD,0x15A,0x197,0x00D,0x01A,0x034,0x068,0x0D0,0x1A0,0x063,0x0C6,0x18C,0x03B,0x076,0x0EC};

  //Currently all channels are used, but the list should be
  //obtained from the PHY
  //The monotonically increasing array currently increases until 26 and then
  //all subsequent positions are filled with 26

  uint8_t start_ch = 11;
  for (unsigned char i = 0;i<sequenceLength;i++)
    {
       chtmpl.m_macHoppingSequenceList[i] = start_ch;
      if (start_ch < 26)
        {
          start_ch++;
        }
      SHUFFLE[i] = LFSR_OUTPUT[i] % sequenceLength;
    }

  for (unsigned char i = 0;i<sequenceLength;i++)
    {
      uint8_t aux = chtmpl.m_macHoppingSequenceList[i];
      chtmpl.m_macHoppingSequenceList[i] = chtmpl.m_macHoppingSequenceList[SHUFFLE[i]];
      chtmpl.m_macHoppingSequenceList[SHUFFLE[i]] = aux;
    }

  /*
  uint8_t uniq_ch = 15;
  for (unsigned char i = 0;i<sequenceLength;i++)
    {
       chtmpl.m_macHoppingSequenceList[i] = uniq_ch;
    }
*/

  chtmpl.m_macHoppingSequenceLength = sequenceLength;

  chtmpl.m_macCurrentHop = chtmpl.m_macHoppingSequenceList.begin();
  chtmpl.m_hopDwellTime = 0;

  def_MacChannelHopping = chtmpl;
}

void
LrWpanTschMac::PrintChannelHoppingList(std::ostream &os)
{
  os << "Channel hopping list for device " << m_shortAddress << std::endl;
  for (unsigned char i = 0; i < def_MacChannelHopping.m_macHoppingSequenceLength ; i++)
    {
      os << "CH[" << (int)i << "] = " << (int)def_MacChannelHopping.m_macHoppingSequenceList[i] << std::endl;
    }
}

void
LrWpanTschMac::SetHoppingSequence(std::vector<uint8_t> sequence, uint8_t id)
{
  LrWpanMacChannelHopping chtmpl;

  chtmpl.m_macHoppingSequenceID = id;
  chtmpl.m_macChannelPage = 0;
  chtmpl.m_macNumberOfChannels = 16;
  chtmpl.m_macPhyConfiguration = 8;
  chtmpl.m_macExtendedBitmap.clear();
  chtmpl.m_macHoppingSequenceList = sequence;
  chtmpl.m_macHoppingSequenceLength = sequence.size();
  chtmpl.m_macCurrentHop = chtmpl.m_macHoppingSequenceList.begin();
  chtmpl.m_hopDwellTime = 0;

  def_MacChannelHopping = chtmpl;
}

void
LrWpanTschMac::ResetMacTimeslotTemplate ()
{
  LrWpanMacTimeslotTemplate timeslottemplate;
  timeslottemplate.m_macTimeslotTemplateId = 0;
  //timeslottemplate.m_macTsCCAOffset = 3680;
 timeslottemplate.m_macTsCCAOffset = 1800;
  timeslottemplate.m_macTsCCA = 128;
//  timeslottemplate.m_macTsTxOffset = 4000;
  timeslottemplate.m_macTsTxOffset = 2120;
//  timeslottemplate.m_macTsRxOffset = 2700;
  timeslottemplate.m_macTsRxOffset = 1120;
//  timeslottemplate.m_macTsRxAckDelay = 4106;
 timeslottemplate.m_macTsRxAckDelay = 800;
//  timeslottemplate.m_macTsTxAckDelay = 4606;
  timeslottemplate.m_macTsTxAckDelay = 1000;
//  timeslottemplate.m_macTsRxWait = 2600;
  timeslottemplate.m_macTsRxWait = 2200;
//  timeslottemplate.m_macTsAckWait = 1000;
  timeslottemplate.m_macTsAckWait = 400;
  timeslottemplate.m_macTsRxTx = 192;
  timeslottemplate.m_macTsMaxAck = 2400;
  timeslottemplate.m_macTsMaxTx = 4256;
  timeslottemplate.m_macTsTimeslotLength = 10000;
//  timeslottemplate.m_macTsTimeslotLength = 15000;

  def_MacTimeslotTemplate = timeslottemplate;
}

void
LrWpanTschMac::GetPhylinkInformation (double m_receivedPower)
{
  m_currentReceivedPower = m_receivedPower;
  m_macLinkInformation(m_macRxID, m_macTxID, m_currentChannel, m_currentReceivedPower, m_currentFadingBias);
}

Ptr<Packet>
LrWpanTschMac::FindTxPacketInEmptySlot (Mac16Address dstAddr)
{
   NS_LOG_FUNCTION(this);
   Ptr<Packet> TxPacket = Create<Packet> (0);
   m_txLinkSequence = 0;

   for (std::deque<TxQueueLinkElement*>::iterator i = m_txQueueAllLink.begin();i != m_txQueueAllLink.end();i++) {
       if ((*i)->txDstAddr == dstAddr){
           if (m_sharedLink  &&  ((*i)->txQueuePerLink.front()->txRequestCW != 0)){
             (*i)->txQueuePerLink.front()->txRequestCW = (*i)->txQueuePerLink.front()->txRequestCW - 1;
              NS_LOG_DEBUG("Find but cannot transmit packet in queue with link position:"<< m_txLinkSequence);
             }
           else{
               TxPacket = (*i)->txQueuePerLink.front()->txQPkt->Copy ();
               m_emptySlot = false;
               break;
             }
         }
       m_txLinkSequence++;
     }

   if (!m_emptySlot){
       NS_LOG_DEBUG("Find Tx packet in queue with link position = " << m_txLinkSequence <<" with queue size = "
                    << m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.size());
    }
   else{
       m_txLinkSequence = 0;
       NS_LOG_DEBUG("Fail to find Tx packet in queues. Empty slot confirmed");
    }

   return TxPacket;
}

void
LrWpanTschMac::HandleTxFailure ()
{
  if (m_sharedLink){
      NS_LOG_DEBUG("Shared Link Failure!");
      if (m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.front()->txRequestNB > 0
          && m_txQueueAllLink[m_txLinkSequence]->txLinkBE < m_macTschPIBAttributes.macMaxBE){
            m_txQueueAllLink[m_txLinkSequence]->txLinkBE++;
        }

      uint8_t txBE = m_txQueueAllLink[m_txLinkSequence]->txLinkBE;
      NS_LOG_DEBUG("Backoff exponent for this shared link is:"<< (int)txBE);

      uint8_t upperBound = (uint8_t) pow (2, txBE) - 1;
      m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.front()->txRequestCW  = (uint8_t)m_random->GetInteger (0, upperBound);
      NS_LOG_DEBUG("Backoff timeslots for this request in the shared link is:"
                   << (int)m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.front()->txRequestCW);

    }

  m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.front()->txRequestNB++;
  NS_LOG_DEBUG ("Increment Retries for the top packet in the queue with link position = "<< m_txLinkSequence);

  if (m_txQueueAllLink[m_txLinkSequence]->txQueuePerLink.front()->txRequestNB == m_macMaxFrameRetries){
      RemoveTxQueueElement();
    }
}

} // namespace ns3
