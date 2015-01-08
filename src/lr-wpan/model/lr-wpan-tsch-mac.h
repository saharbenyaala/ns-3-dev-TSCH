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
 *  Luis Pacheco <luisbelem@gmail.com>
 *  Peishuo Li <pressthunder@gmail.com>
 *  Peter Kourzanov <peter.kourzanov@gmail.com>
 */
#ifndef LR_WPAN_TSCH_MAC_H
#define LR_WPAN_TSCH_MAC_H

#include <ns3/object.h>
#include <ns3/traced-callback.h>
#include <ns3/mac16-address.h>
#include <ns3/mac64-address.h>
#include <ns3/sequence-number.h>
#include <ns3/lr-wpan-phy.h>
#include <ns3/lr-wpan-mac.h>
#include <ns3/lr-wpan-mac-header.h>
#include <ns3/traced-value.h>
#include <ns3/event-id.h>
#include <deque>
#include <bitset>


namespace ns3 {

class Packet;
//class LrWpanCsmaCa; //not supported at the moment

/**
 * \defgroup lr-wpan LR-WPAN models
 *
 * This section documents the API of the IEEE 802.15.4-related models.  For a generic functional description, please refer to the ns-3 manual.
 */
typedef enum
{
  TSCH_MAC_IDLE,
  TSCH_MAC_CCA,
  TSCH_MAC_SENDING,
  TSCH_MAC_ACK_PENDING,
  TSCH_MAC_ACK_PENDING_END,
  TSCH_CHANNEL_ACCESS_FAILURE,
  TSCH_CHANNEL_IDLE,
  TSCH_SET_PHY_TX_ON,
  TSCH_MAC_RX,
  TSCH_PKT_WAIT_END
} LrWpanTschMacState;


struct LrWpanFrameControlOptions
{
  LrWpanFrameControlOptions ()
    : m_PanIdSupressed(0),
        IesIncluded(0),
        SeqNSupressed(0)
     {

  }
  bool m_PanIdSupressed; 
  bool IesIncluded;
  bool SeqNSupressed;
};

struct TschMcpsDataRequestParams
{
  TschMcpsDataRequestParams ()
    : m_srcAddrMode (SHORT_ADDR),
      m_dstAddrMode (SHORT_ADDR),
      m_dstPanId (0),
      m_dstAddr (),
      m_msduHandle (0),
      m_ACK_TX(0),
      m_GTSTX(0),
      m_IndirectTx(0),
      m_sendMultipurpose(0)
  {
  }
  LrWpanAddressMode m_srcAddrMode;
  LrWpanAddressMode m_dstAddrMode;
  uint16_t m_dstPanId;
  Mac16Address m_dstAddr;
  uint8_t m_msduHandle;
  //uint8_t m_txOptions;  // bitmap

  bool m_ACK_TX;
  bool m_GTSTX;
  bool m_IndirectTx;
  uint8_t m_SecurityLevel;
  uint8_t m_KeyIdMode;
  std::vector<uint8_t> m_KeySource;
  uint8_t m_KeyIndex;

  //802.15.4e

  LrWpanFrameControlOptions m_frameControlOptions;
  std::vector<uint8_t> m_headerIElist;
  std::vector<uint8_t> m_payloadIElist;
  bool m_sendMultipurpose;
};


typedef struct
{
  uint8_t macMinBE;
  uint8_t macMaxBE;
  uint32_t m_macDisconnectTime;
  uint32_t m_macJoinPriority;
  uint64_t m_macASN; //Need to store 80bits!
  bool m_macNoHLBuffers;
}LrwpanMacTschPibAttributes;

typedef enum {
  NORMAL = 0,
  ADVERTISING = 1
}LrWpanMacTschLinkType;

typedef struct {
  uint16_t m_macLinkHandle;
  uint8_t m_macLinkOptions;
  LrWpanMacTschLinkType m_macLinkType;
  uint8_t m_SlotframeHandle;
  Mac16Address m_macNodeAddress;
  uint16_t m_macTimeslot;
  uint16_t m_macChannelOffset;
} LrWpanMacLinkTable;

typedef struct {
  //Table 52e—TSCH-MAC PIB attributes for macTimeslotTemplate
  uint8_t m_macTimeslotTemplateId;
  uint16_t m_macTsCCAOffset;
  uint16_t m_macTsCCA;
  uint16_t m_macTsTxOffset;
  uint16_t m_macTsRxOffset;
  uint16_t m_macTsRxAckDelay;
  uint16_t m_macTsTxAckDelay;
  uint16_t m_macTsRxWait;
  uint16_t m_macTsAckWait;
  uint16_t m_macTsRxTx;
  uint16_t m_macTsMaxAck;
  uint16_t m_macTsMaxTx;
  uint16_t m_macTsTimeslotLength;
}LrWpanMacTimeslotTemplate;

typedef struct {
  //Table 52f—MAC PIB attributes for Hopping Sequence
  uint8_t m_macHoppingSequenceID;
  uint8_t m_macChannelPage;
  uint16_t m_macNumberOfChannels;
  uint32_t m_macPhyConfiguration;
  std::vector<bool> m_macExtendedBitmap; //size varies, has to be allocated in run time
  uint16_t m_macHoppingSequenceLength;
  std::vector<uint8_t> m_macHoppingSequenceList;
  std::vector<uint8_t>::iterator m_macCurrentHop;
  uint16_t m_hopDwellTime;
}LrWpanMacChannelHopping;
/*
 * End of TSCH specific MAC PIB attributes
 */


/*********************************************************************
 * START MLME-TSCH params and enums
 ********************************************************************/
typedef enum {
  MlmeTschMode_ON = 1,
  MlmeTschMode_OFF = 2
}LrWpanMlmeTschMode;

struct MlmeTschModeRequestParams {
  LrWpanMlmeTschMode TSCHMode;
};

typedef enum {
  LrWpanMlmeTschModeConfirmStatus_SUCCESS = 1,
  LrWpanMlmeTschModeConfirmStatus_NO_SYNC = 2
}LrWpanMlmeTschModeConfirmStatus;

struct MlmeTschModeConfirmParams {
  LrWpanMlmeTschMode TSCHMode;
  LrWpanMlmeTschModeConfirmStatus Status;
};

typedef enum {
  MlmeSlotframeOperation_ADD = 1,
  MlmeSlotframeOperation_DELETE = 2,
  MlmeSlotframeOperation_MODIFY = 3
}LrWpanMlmeSlotframeOperation;

struct MlmeSetSlotframeRequestParams {
  uint8_t slotframeHandle;
  LrWpanMlmeSlotframeOperation Operation;
  uint16_t size;
};

struct MacPibSlotframeAttributes {
  uint8_t slotframeHandle;
  uint16_t size;
};

typedef enum {
  MlmeSetLinkRequestlinkType_NORMAL = 0, 
  MlmeSetLinkRequestlinkType_ADVERTISING = 1
}LrWpanMlmeSetLinkRequestlinkType;

struct MacPibLinkAttributes {
  uint16_t macLinkHandle;
  std::bitset<8> macLinkOptions; //b0 = Transmit, b1 = Receive, b2 = Shared, b3= Timekeeping, b4–b7 reserved.
  LrWpanMlmeSetLinkRequestlinkType macLinkType;
  uint8_t slotframeHandle;
  Mac16Address macNodeAddr; //not using Mac16_Address because 0xffff means the link can be used for frames destined for the broadcast address
  uint16_t macTimeslot; //refer to 5.1.1.5
  uint16_t macChannelOffset; //refer to 5.1.1.5.3
  double* macLinkFadingBias; //1D bias coefficient array which describe the multi-path effect of specific link on all channels
  uint32_t macTxID;
  uint32_t macRxID;
};

typedef enum  {
  MlmeSetSlotframeConfirmStatus_SUCCESS = 1,
  MlmeSetSlotframeConfirmStatus_INVALID_PARAMETER = 2,
  MlmeSetSlotframeConfirmStatus_SLOTFRAME_NOT_FOUND = 3, 
  MlmeSetSlotframeConfirmStatus_MAX_SLOTFRAMES_EXCEEDED = 4
}LrWpanMlmeSetSlotframeConfirmStatus;

struct MlmeSetSlotframeConfirmParams {
  uint8_t slotframeHandle;
  LrWpanMlmeSetSlotframeConfirmStatus Status;
};

struct LrWpanSlotframe{
  uint8_t slotframeHandle;
  std::list<LrWpanMacTimeslotTemplate> timeslots;
  uint16_t size;
} ;

typedef enum  {
  MlmeSetLinkRequestOperation_ADD_LINK = 1,
  MlmeSetLinkRequestOperation_DELETE_LINK = 2,
  MlmeSetLinkRequestOperation_MODIFY_LINK = 3
}LrWpanMlmeSetLinkRequestOperation;



struct MlmeSetLinkRequestParams {
  LrWpanMlmeSetLinkRequestOperation Operation;
  uint16_t linkHandle;
  uint8_t slotframeHandle;
  uint16_t Timeslot; //refer to 5.1.1.5
  uint16_t ChannelOffset; //refer to 5.1.1.5.3
  std::bitset<8> linkOptions; //b0 = Transmit, b1 = Receive, b2 = Shared, b3= Timekeeping, b4–b7 reserved.
  LrWpanMlmeSetLinkRequestlinkType linkType;
  Mac16Address nodeAddr; //not using Mac16_Address because 0xffff means the link can be used for frames destined for the broadcast address
  double* linkFadingBias; //1D bias coefficient array which describe the multi-path effect of specific link on all channels
  uint32_t TxID;
  uint32_t RxID;
};

typedef enum {  
  MlmeSetLinkConfirmStatus_SUCCESS = 0, 
  MlmeSetLinkConfirmStatus_INVALID_PARAMETER = 1, 
  MlmeSetLinkConfirmStatus_UNKNOWN_LINK = 2, 
  MlmeSetLinkConfirmStatus_MAX_LINKS_EXCEEDED = 3
}LrWpanMlmeSetLinkConfirmStatus;

struct MlmeSetLinkConfirmParams {
  LrWpanMlmeSetLinkConfirmStatus Status;
  uint16_t linkHandle;
  uint8_t slotframeHandle;
};

struct MlmeKeelAliveRequestParams{
  Mac16Address dstAddr;
  uint16_t keepAlivePeriod;
};

typedef enum {
  MlmeKeepAliveConfirmStatus_SUCCESS = 0,
  MlmeKeepAliveConfirmStatus_INVALID_PARAMETER = 1
}LrWpanMlmeKeepAliveConfirmStatus;

struct MlmeKeepAliveConfirmParams {
  LrWpanMlmeKeepAliveConfirmStatus Staus;
};

 /*********************************************************************
 * END MLME-TSCH params and enums
 ********************************************************************/

struct TschCurrentLink{
  uint8_t slotframeHandle;
  uint16_t linkHandle;
  bool active;
};


// This callback is called after a MlmeSetSlotframe has been called from
// the higher layer.  It returns a status of the outcome of the
// request
typedef Callback<void, MlmeSetSlotframeConfirmParams> MlmeSetSlotframeConfirmCallback;

// This callback is called after a MlmeTschMode has been called from
// the higher layer.  It returns a status of the outcome of the
// request
typedef Callback<void, MlmeTschModeConfirmParams> MlmeTschModeConfirmCallback;

// This callback is called after a MlmeSetLink has been called from
// the higher layer.  It returns a status of the outcome of the
// request
typedef Callback<void, MlmeSetLinkConfirmParams> MlmeSetLinkConfirmCallback;


// This callback is called after a McpsDataRequest has been called from
// the higher layer.  It returns a status of the outcome of the
// transmission request
typedef Callback<void, McpsDataConfirmParams> McpsDataConfirmCallback;

// This callback is called after a Mcps has successfully received a
// frame and wants to deliver it to the higher layer.
//
// XXX for now, we do not deliver all of the parameters in section
// 7.1.1.3.1 but just send up the packet.
typedef Callback<void, McpsDataIndicationParams, Ptr<Packet> > McpsDataIndicationCallback;


/**
 * \ingroup lr-wpan
 *
 * Class that implements the LR-WPAN TSCH Mac state machine
 */
class LrWpanTschMac : public LrWpanMac //Object
{
public:
  /**
   * Get the type ID.
   *
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * The minimum number of octets added by the MAC sublayer to the PSDU.
   * See IEEE 802.15.4-2006, section 7.4.1, Table 85
   */
  static const uint32_t aMinMPDUOverhead;

  /**
   * Default constructor.
   */
  LrWpanTschMac (void);
  virtual ~LrWpanTschMac (void);

  // XXX these setters will become obsolete if we use the attribute system
  /**
   * Set the short address of this MAC.
   *
   * \param address the new address
   */
  void SetShortAddress (Mac16Address address);

  /**
   * Get the short address of this MAC.
   *
   * \return the short address
   */
  Mac16Address GetShortAddress (void) const;

  /**
   * Set the extended address of this MAC.
   *
   * \param address the new address
   */
  void SetExtendedAddress (Mac64Address address);

  /**
   * Get the extended address of this MAC.
   *
   * \return the extended address
   */
  Mac64Address GetExtendedAddress (void) const;

  /**
   * Set the PAN id used by this MAC.
   *
   * \param panId the new PAN id.
   */
  void SetPanId (uint16_t panId);

  /**
   * Get the PAN id used by this MAC.
   *
   * \return the PAN id.
   */
  uint16_t GetPanId (void) const;

  /**
   *  IEEE 802.15.4-2006, section 7.1.1.1
   *  MCPS-DATA.request
   *  Request to transfer a MSDU.
   *
   *  \param params the request parameters
   *  \param p the packet to be transmitted
   */
  void McpsDataRequest (TschMcpsDataRequestParams params, Ptr<Packet> p);

  /**
   * Set the CSMA/CA implementation to be used by the MAC.
   *
   * \param csmaCa the CSMA/CA implementation
   */
  //void SetCsmaCa (Ptr<LrWpanCsmaCa> csmaCa);

  /**
   * Set the underlying PHY for the MAC.
   *
   * \param phy the PHY
   */
  void SetPhy (Ptr<LrWpanPhy> phy);

  /**
   * Get the underlying PHY of the MAC.
   *
   * \return the PHY
   */
  Ptr<LrWpanPhy> GetPhy (void);

  /**
   * Set the callback for the indication of an incoming data packet.
   * The callback implements MCPS-DATA.indication SAP of IEEE 802.15.4-2006,
   * section 7.1.1.3.
   *
   * \param c the callback
   */
  void SetMcpsDataIndicationCallback (McpsDataIndicationCallback c);

  /**
   * Set the callback for the confirmation of a data transmission request.
   * The callback implements MCPS-DATA.confirm SAP of IEEE 802.15.4-2006,
   * section 7.1.1.2.
   *
   * \param c the callback
   */
  void SetMcpsDataConfirmCallback (McpsDataConfirmCallback c);

  //TSCH management
  /**
   * Set the callback for the confirmation of a set slotframe request.
   * The callback implements MLME-SET-SLOTFRAME.confirm SAP of IEEE 802.15.4e-2012,
   * section 6.2.19.2
   *
   * \param c the callback
   */
  void SetMlmeSetSlotframeConfirmCallback (MlmeSetSlotframeConfirmCallback c);

  /**
   * Set the callback for the confirmation of a set link request.
   * The callback implements MLME-SET-LINK.confirm SAP of IEEE 802.15.4e-2012,
   * section 6.2.19.4
   *
   * \param c the callback
   */
  void SetMlmeSetLinkConfirmCallback (MlmeSetLinkConfirmCallback c);

  /**
   * Set the callback for the confirmation of a tsch mode request.
   * The callback implements MLME-TSCH-MODE.confirm SAP of IEEE 802.15.4e-2012,
   * section 6.2.19.6
   *
   * \param c the callback
   */
  void SetMlmeTschModeConfirmCallback (MlmeTschModeConfirmCallback c);


  // interfaces between MAC and PHY
  /**
   *  IEEE 802.15.4-2006 section 6.2.1.3
   *  PD-DATA.indication
   *  Indicates the transfer of an MPDU from PHY to MAC (receiving)
   *  @param psduLength number of bytes in the PSDU
   *  @param p the packet to be transmitted
   *  @param lqi Link quality (LQI) value measured during reception of the PPDU
   */
  void PdDataIndication (uint32_t psduLength, Ptr<Packet> p, uint8_t lqi);

  /**
   *  IEEE 802.15.4-2006 section 6.2.1.2
   *  Confirm the end of transmission of an MPDU to MAC
   *  @param status to report to MAC
   *  PHY PD-DATA.confirm status
   */
  void PdDataConfirm (LrWpanPhyEnumeration status);

  /**
   *  IEEE 802.15.4-2006 section 6.2.2.2
   *  PLME-CCA.confirm status
   *  @param status TRX_OFF, BUSY or IDLE
   */
  void PlmeCcaConfirm (LrWpanPhyEnumeration status);

  /**
   *  IEEE 802.15.4-2006 section 6.2.2.4
   *  PLME-ED.confirm status and energy level
   *  @param status SUCCESS, TRX_OFF or TX_ON
   *  @param energyLevel 0x00-0xff ED level for the channel
   */
  void PlmeEdConfirm (LrWpanPhyEnumeration status, uint8_t energyLevel);

  /**
   *  IEEE 802.15.4-2006 section 6.2.2.6
   *  PLME-GET.confirm
   *  Get attributes per definition from Table 23 in section 6.4.2
   *  @param status SUCCESS or UNSUPPORTED_ATTRIBUTE
   *  @param id the attributed identifier
   *  @param attribute the attribute value
   */
  void PlmeGetAttributeConfirm (LrWpanPhyEnumeration status,
                                LrWpanPibAttributeIdentifier id,
                                LrWpanPhyPibAttributes* attribute);

  /**
   *  IEEE 802.15.4-2006 section 6.2.2.8
   *  PLME-SET-TRX-STATE.confirm
   *  Set PHY state
   *  @param status in RX_ON,TRX_OFF,FORCE_TRX_OFF,TX_ON
   */
  void PlmeSetTRXStateConfirm (LrWpanPhyEnumeration status);

  /**
   *  IEEE 802.15.4-2006 section 6.2.2.10
   *  PLME-SET.confirm
   *  Set attributes per definition from Table 23 in section 6.4.2
   *  @param status SUCCESS, UNSUPPORTED_ATTRIBUTE, INVALID_PARAMETER, or READ_ONLY
   *  @param id the attributed identifier
   */
  void PlmeSetAttributeConfirm (LrWpanPhyEnumeration status,
                                LrWpanPibAttributeIdentifier id);

  /**
   * CSMA-CA algorithm calls back the MAC after executing channel assessment.
   *
   * \param macState indicate BUSY oder IDLE channel condition
   */
  void SetLrWpanMacState (LrWpanTschMacState macState);

  /**
   * Get the current association status.
   *
   * \return current association status
   */
  LrWpanAssociationStatus GetAssociationStatus (void) const;

  /**
   * Set the current association status.
   *
   * \param status new association status
   */
  void SetAssociationStatus (LrWpanAssociationStatus status);

  //TSCH methods
  /**
   * MLME-TSCH-MODE.request, put the MAC into or out of the TSCH mode
   */
  void MlmeTschModeRequest (MlmeTschModeRequestParams params);

  /**
   * MLME-SET-SLOTFRAME.request primitive is used to add, delete, or modify a slotframe at the MAC sublayer
   */
  void MlmeSetSlotframeRequest (MlmeSetSlotframeRequestParams params);

  /**
   * The MLME-SET-LINK.request primitive requests to add a new link, or delete or modify an existing link at
   * the MAC sublayer.
   */
  void MlmeSetLinkRequest (MlmeSetLinkRequestParams params);

  //MAC sublayer constants
  uint64_t m_aMaxMACPayloadSize;                // aMaxPHYPacketSize – aMinMPDUOverhead


  /**
   * Set default hopping sequence
   */
  void SetDefaultHoppingSequence(uint16_t);

  /**
   * Print channel hopping list
   */
  void PrintChannelHoppingList(std::ostream &os);

  /**
   * Set channel hopping sequence
   */
  void SetHoppingSequence(std::vector<uint8_t> sequence, uint8_t id);

  /**
   * TSCH MAC CCA enable state
   */
  bool m_macCCAEnabled;

  /**
   * Set TSCH MAC CCA enabled
   */
  void SetMacCCAEnabled(bool);

  /**
   * Get TSCH MAC CCA enable state
   */
  bool GetMacCCAEnables();

  //MAC sublayer constants
  //MAC PIB attributes
  /**
   * Indicates if MAC sublayer is in receive all mode. True mean accept all
   * frames from PHY.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  bool m_macPromiscuousMode;

  /**
   * 16 bits id of PAN on which this device is operating. 0xffff means not
   * asscoiated.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  uint16_t m_macPanId;

  /**
   * Sequence number added to transmitted data or MAC command frame, 00-ff.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  SequenceNumber8 m_macDsn;

  /**
   * The maximum number of retries allowed after a transmission failure.
   * See IEEE 802.15.4-2006, section 7.4.2, Table 86.
   */
  uint8_t m_macMaxFrameRetries;

  /**
   * Get the macMaxFrameRetries attribute value.
   *
   * \return the maximum number of retries
   */
  uint8_t GetMacMaxFrameRetries (void) const;

  /**
   * Set the macMaxFrameRetries attribute value.
   *
   * \param retries the maximum number of retries
   */
  void SetMacMaxFrameRetries (uint8_t retries);

  void GetPhylinkInformation (double m_receivedPower);

protected:
  // Inherited from Object.
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

private:
  /**
   * Helper structure for managing transmission queue elements.
   */
  struct TxQueueRequestElement
  {
    uint8_t txQMsduHandle;
    Ptr<Packet> txQPkt;
    uint8_t txRequestNB;
    uint8_t txRequestCW;
  };

  struct TxQueueLinkElement
  {
    std::deque<TxQueueRequestElement*> txQueuePerLink;
    Mac16Address txDstAddr;
    uint8_t txLinkBE;
  };

  /**
   * Send an acknowledgment packet for the given sequence number.
   *
   * \param seqno the sequence number for the ACK
   */
  void SendAck (uint8_t seqno, bool seqnumsup);

  /**
   * Remove the tip of the transmission queue, including clean up related to the
   * last packet transmission.
   */

  void RemoveTxQueueElement ();

  /**
   * Change the current MAC state to the given new state.
   *
   * \param newState the new state
   */
  void ChangeMacState (LrWpanTschMacState newState);

  /**
   * The trace source fired when packets are considered as successfully sent
   * or the transmission has been given up.
   * Only non-broadcast packets are traced.
   *
   * The data should represent:
   * packet, number of retries, total number of csma backoffs(not included in tsch)
   *
   * \see class CallBackTraceSource
   */
  //TracedCallback<Ptr<const Packet>, uint8_t, uint8_t > m_sentPktTrace;
  TracedCallback<Ptr<const Packet>, uint8_t > m_sentPktTrace;

  /**
   * The trace source fired when packets come into the "top" of the device
   * at the L3/L2 transition, when being queued for transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxEnqueueTrace;

  /**
   * The trace source fired when packets are dequeued from the
   * L3/l2 transmission queue.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxDequeueTrace;

  /**
   * The trace source fired when packets are being sent down to L1.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxTrace;

  TracedCallback<Ptr<const Packet> > m_macMaxRetries;

  /**
   * The trace source fired when packets where successfully transmitted, that is
   * an acknowledgment was received, if requested, or the packet was
   * successfully sent by L1, if no ACK was requested.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxOkTrace;

  /**
   * The trace source fired when packets are dropped due to missing ACKs or
   * because of transmission failures in L1.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macTxDropTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3
   * transition).  This is a promiscuous trace.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macPromiscRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * immediately before being forwarded up to higher layers (at the L2/L3
   * transition).  This is a non-promiscuous trace.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macRxTrace;

  /**
   * The trace source fired for packets successfully received by the device
   * but dropped before being forwarded up to higher layers (at the L2/L3
   * transition).
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_macRxDropTrace;

  /**
   * A trace source that emulates a non-promiscuous protocol sniffer connected
   * to the device.  Unlike your average everyday sniffer, this trace source
   * will not fire on PACKET_OTHERHOST events.
   *
   * On the transmit size, this trace hook will fire after a packet is dequeued
   * from the device queue for transmission.  In Linux, for example, this would
   * correspond to the point just before a device hard_start_xmit where
   * dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET
   * ETH_P_ALL handlers.
   *
   * On the receive side, this trace hook will fire when a packet is received,
   * just before the receive callback is executed.  In Linux, for example,
   * this would correspond to the point at which the packet is dispatched to
   * packet sniffers in netif_receive_skb.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_snifferTrace;

  /**
   * A trace source that emulates a promiscuous mode protocol sniffer connected
   * to the device.  This trace source fire on packets destined for any host
   * just like your average everyday packet sniffer.
   *
   * On the transmit size, this trace hook will fire after a packet is dequeued
   * from the device queue for transmission.  In Linux, for example, this would
   * correspond to the point just before a device hard_start_xmit where
   * dev_queue_xmit_nit is called to dispatch the packet to the PF_PACKET
   * ETH_P_ALL handlers.
   *
   * On the receive side, this trace hook will fire when a packet is received,
   * just before the receive callback is executed.  In Linux, for example,
   * this would correspond to the point at which the packet is dispatched to
   * packet sniffers in netif_receive_skb.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_promiscSnifferTrace;

  /*
   * A trace source that fires when the LrWpanMac changes states.
   * Parameters are the old mac state and the new mac state.
   */
  TracedCallback<LrWpanTschMacState, LrWpanTschMacState> m_macStateLogger;

  /**
   * The PHY associated with this MAC.
   */
  Ptr<LrWpanPhy> m_phy;

  /**
   * The CSMA/CA implementation used by this MAC.
   */
  //Ptr<LrWpanCsmaCa> m_csmaCa;

  /**
   * This callback is used to notify incoming packets to the upper layers.
   * See IEEE 802.15.4-2006, section 7.1.1.3.
   */
  McpsDataIndicationCallback m_mcpsDataIndicationCallback;

  /**
   * This callback is used to report data transmission request status to the
   * upper layers.
   * See IEEE 802.15.4-2006, section 7.1.1.2.
   */
  McpsDataConfirmCallback m_mcpsDataConfirmCallback;

  //TSCH callbacks
  /**
   * This callback is used to report TSCH slotframe setting status to the
   * upper layers.
   * See IEEE 802.15.4e-2012 section 5.1.2.6.
   */
  MlmeSetSlotframeConfirmCallback m_mlmeSetSlotframeConfirmCallback;

  /**
   * This callback is used to report TSCH mode status to the
   * upper layers.
   * See IEEE 802.15.4e-2012 section 5.1.2.6.
   */
  MlmeTschModeConfirmCallback m_mlmeTschModeConfirmCallback;

  /**
   * This callback is used to report TSCH link setting status to the
   * upper layers.
   * See IEEE 802.15.4e-2012 section 5.1.2.6.
   */
  MlmeSetLinkConfirmCallback m_mlmeSetLinkConfirmCallback;

  /**
   * The current state of the MAC layer.
   */
  LrWpanTschMacState m_lrWpanMacState;

  LrWpanTschMacState m_lrWpanMacStatePending;

  /**
   * The current association status of the MAC layer.
   */
  LrWpanAssociationStatus m_associationStatus;

  /**
   * The packet which is currently being sent by the MAC layer.
   */
  Ptr<Packet> m_txPkt;  // XXX need packet buffer instead of single packet

  /**
   * The short address used by this MAC. Currently we do not have complete
   * extended address support in the MAC, nor do we have the association
   * primitives, so this address has to be configured manually.
   */
  Mac16Address m_shortAddress;

  /**
   * The extended address used by this MAC. Extended addresses are currently not
   * really supported.
   */
  Mac64Address m_selfExt;

  /**
   * The transmit queue used by the MAC.
   */
  std::deque<TxQueueLinkElement*> m_txQueueAllLink;

  /**
   * Scheduler event for a deferred MAC state change.
   */
  EventId m_setMacState;



  /* TSCH methods */
  /**
   * Reset MAC PIB specified for TSCH to default values
   * See IEEE 802.15.4e-2012 section 6.4.4.3, Table 52b
   */
  void ResetMacTschPibAttributes();

  /**
   * Reset MACTimeslotTemplate attributes to default values
   * See IEEE 802.15.4e-2012 section 6.4.3.3.3, Table 52e
   */
  void ResetMacTimeslotTemplate();

  /**
   * Schedule the channle hopping of the slotframe with given handle and size
   * See IEEE 802.15.4e-2012 section 5.1.1.5
   */
  void ScheduleTimeslot(uint8_t handle, uint16_t size);

  /**
   * Increment ASN of timeslots, schedule next incrementation, reset MAC state to be TSCH_MAC_IDLE if not, set next link if requested
   */
  void IncAsn();

  /**
   * Set MAC state to be TSCH_MAC_ACK_PENDING
   */
  void WaitAck();

  /**
   * Set MAC state to be TSCH_MAC_ACK_PENDING_END if no ACK received after setted waiting time for ACK (m_macTsAckWait)
   */
  void AckWaitDone();

  /**
   * Set MAC state to be TSCH_PKT_WAIT_END if no expected packet received after setted waiting time for transmitted frame (m_macTsRxWait)
   */
  void RxWaitDone();

  void HandleTxFailure();

  void SetTxLinkQueue(TxQueueRequestElement * newRequestElement, Mac16Address newDstAddr, uint8_t newSeqNum);

  Ptr<Packet> FindTxPacketInEmptySlot(Mac16Address dstAddr);

  /**
   * Pending packet size
   */
  uint32_t m_latestPacketSize;

  /**
   * Current Radio Channel
   */
  uint8_t m_currentChannel;

  uint32_t m_macTxID;

  uint32_t m_macRxID;

  uint32_t m_txLinkSequence;

  double m_currentReceivedPower;

  double m_currentFadingBias;

  /**
   * Hopping enable state
   */
  bool m_macHoppingEnabled;

  bool m_waitingLink;

  bool m_sharedLink;

  bool m_emptySlot;

  bool m_newSlot;

  /**
   * Timestamp of waiting time finishing for ACK or transmitted frame
   */
  Time m_macWaitDone;

  /**
   * Timestamp of pending frame transmission start
   */
  Time m_lastTransmission;

  Ptr<UniformRandomVariable> m_random;

  /**
   * PIB attributes of Hopping Sequence
   */
  LrWpanMacChannelHopping def_MacChannelHopping;

  /**
   * Current TSCH Link
   */
  TschCurrentLink currentLink;

  /**
   * Requested link parameters
   */
  MlmeSetLinkRequestParams m_waitingLinkParams;

  /**
   * PIB attributes of TimeSlotTemplate
   */
  LrWpanMacTimeslotTemplate def_MacTimeslotTemplate;

  /**
   * List of TSCH slotframe MAC PIB attributes
   */
  std::list<MacPibSlotframeAttributes> m_macSlotframeTable;

  /**
   * List of TSCH link MAC PIB attributes
   */
  std::list<MacPibLinkAttributes> m_macLinkTable;

  /**
   * List of TSCH specified MAC PIB attributes
   */
  LrwpanMacTschPibAttributes m_macTschPIBAttributes;

  //TSCH traces
  /**
   * TSCH traces of different timeslot types
   *
   * \see class CallBackTraceSource
   */

  /**
   * The trace source fired when device receives a data packet and sends an ACK
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<uint32_t> m_macRxDataTxAckTrace;

  /**
   * The trace source fired when device sends a data packet
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<uint32_t> m_macTxDataTrace;

  /**
   * The trace source fired when device receives a data packet
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<uint32_t> m_macRxDataTrace;

  /**
   * The trace source fired when device sends a data packet and receives an ACK
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<uint32_t> m_macTxDataRxAckTrace;

  /**
   * The trace source fired when the timeslot is not assigned for the device
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<uint32_t> m_macSleepTrace;

  /**
   * The trace source fired when device sends a data packet, listens for an ACK but does not receive one
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<uint32_t> m_macWaitAckTrace;

  /**
   * The trace source fired when device performs a CCA and the channel is busy
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<uint32_t> m_macChannelBusyTrace;

  /**
   * The trace source fired when device listens for a packet but does not receive one
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<uint32_t> m_macIdleTrace;

  /**
   * The trace source fired when device has no packet at its buffer
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<uint32_t> m_macRxEmptyBufferTrace;

  TracedCallback<uint32_t, uint32_t, uint8_t, double, double> m_macLinkInformation;
};


} // namespace ns3

#endif /* LR_WPAN_MAC_H */
