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
 * Author: kwong yin <kwong-sang.yin@boeing.com>
 *         Peter Kourzanov <peter.kourzanov@gmail.com>
 */
#include "lr-wpan-mac-header.h"
#include <ns3/address-utils.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LrWpanMacHeader);

// TODO: Test Compressed PAN Id, Security Enabled, different size Key

LrWpanMacHeader::LrWpanMacHeader ()
{
  SetType (LRWPAN_MAC_DATA);     // Assume Data frame
  SetSecDisable ();              // Assume there is No Aux Sec but
  SetNoFrmPend ();               // No Frame Pending
  SetNoAckReq ();                // No Ack Frame will be expected from recepient
  SetNoPanIdComp ();             // No PAN Id Compression since no addresses
  SetFrmCtrlRes (0);             // Initialize the 3 reserved bits to 0
  SetDstAddrMode (NOADDR);       // Assume there will be no src and dst address
  SetSrcAddrMode (NOADDR);
  SetFrameVer (1);               //Indicates an IEEE 802.15.4 frame
  SetNoSeqNumSup();              //No supression of the sequence number
}


LrWpanMacHeader::LrWpanMacHeader (enum LrWpanMacType wpanMacType,
                                  uint8_t seqNum)
{
  SetType (wpanMacType);
  SetSeqNum (seqNum);
  SetSecDisable ();              // Assume there is No Aux Sec but
  SetNoFrmPend ();               // No Frame Pending
  SetNoAckReq ();                // No Ack Frame will be expected from recepient
  SetNoPanIdComp ();             // No PAN Id Compression since no addresses
  SetFrmCtrlRes (0);             // Initialize the 3 reserved bits to 0
  SetDstAddrMode (NOADDR);       // Assume there will be no src and dst address
  SetSrcAddrMode (NOADDR);
  SetFrameVer (1);               //Indicates an IEEE 802.15.4 frame
}


LrWpanMacHeader::~LrWpanMacHeader ()
{
}


enum LrWpanMacHeader::LrWpanMacType
LrWpanMacHeader::GetType (void) const
{
  switch (m_fctrlFrmType)
    {
    case 0:
      return LRWPAN_MAC_BEACON;
      break;
    case 1:
      return LRWPAN_MAC_DATA;
      break;
    case 2:
      return LRWPAN_MAC_ACKNOWLEDGMENT;
      break;
    case 3:
      return LRWPAN_MAC_COMMAND;
      break;
    case 4:
      return LRWPAN_MAC_LLDN;
      break;
    case 5:
      return LRWPAN_MAC_MULTIPURPOSE;
      break;
    default:
      return LRWPAN_MAC_RESERVED;
    }
}



uint16_t
LrWpanMacHeader::GetFrameControl (void) const
{
  uint16_t val = 0;

  val = m_fctrlFrmType & (0x07);                 // Bit 0-2
  val |= (m_fctrlSecU << 3) & (0x01 << 3);         // Bit 3
  val |= (m_fctrlFrmPending << 4) & (0x01 << 4);   // Bit 4
  val |= (m_fctrlAckReq << 5) & (0x01 << 5);        // Bit 5
  val |= (m_fctrlPanIdComp << 6) & (0x01 << 6);    // Bit 6


  val |= (m_fctrlDstAddrMode << 10) & (0x03 << 10); // Bit 10-11
  val |= (m_fctrlFrmVer << 12) & (0x03 << 12);     // Bit 12-13
  val |= (m_fctrlSrcAddrMode << 14) & (0x03 << 14); // Bit 14-15

  //802.15.4e
  if (m_fctrlFrmVer == 2) {

    val |= (m_fctrlReserved << 7) & (0x01 << 7);          // Bit 7
    val |= (m_fctrlSeqNumSuppression << 8) & (0x01 << 8);          // Bit 8
    val |= (m_fctrlIEListPresent << 9) & (0x01 << 9);          // Bit 9
  } else {
    val |= (m_fctrlReserved << 7) & (0x07 << 7);           // Bit 7-9
  }
  return val;

}

bool
LrWpanMacHeader::IsSecEnable (void) const
{
  return (m_fctrlSecU == 1);
}

bool
LrWpanMacHeader::IsFrmPend (void) const
{
  return (m_fctrlFrmPending == 1);
}

bool
LrWpanMacHeader::IsAckReq (void) const
{
  return (m_fctrlAckReq == 1);
}

bool
LrWpanMacHeader::IsPanIdComp (void) const
{
  return (m_fctrlPanIdComp == 1);
}

uint8_t
LrWpanMacHeader::GetFrmCtrlRes (void) const
{
  return (m_fctrlReserved);
}

uint8_t
LrWpanMacHeader::GetDstAddrMode (void) const
{
  return m_fctrlDstAddrMode;
}

uint8_t
LrWpanMacHeader::GetFrameVer (void) const
{
  return m_fctrlFrmVer;
}

uint8_t
LrWpanMacHeader::GetSrcAddrMode (void) const
{
  return m_fctrlSrcAddrMode;
}


uint8_t
LrWpanMacHeader::GetSeqNum (void) const
{
  return(m_SeqNum);
}


uint16_t
LrWpanMacHeader::GetDstPanId (void) const
{
  return(m_addrDstPanId);
}


Mac16Address
LrWpanMacHeader::GetShortDstAddr (void) const
{
  return(m_addrShortDstAddr);
}
Mac64Address
LrWpanMacHeader::GetExtDstAddr (void) const
{
  return(m_addrExtDstAddr);
}

uint16_t
LrWpanMacHeader::GetSrcPanId (void) const
{
  return(m_addrSrcPanId);
}



Mac16Address
LrWpanMacHeader::GetShortSrcAddr (void) const
{
  return(m_addrShortSrcAddr);
}
Mac64Address
LrWpanMacHeader::GetExtSrcAddr (void) const
{
  return(m_addrExtSrcAddr);
}


uint8_t
LrWpanMacHeader::GetSecControl (void) const
{
  uint8_t val = 0;

  val = m_secctrlSecLevel & (0x7);              // Bit 0-2
  val |= (m_secctrlKeyIdMode << 3) & (0x3 << 3);  // Bit 3-4
  val |= (m_secctrlReserved << 5) & (0x7 << 5);   // Bit 5-7

  return(val);
}

uint32_t
LrWpanMacHeader::GetFrmCounter (void) const
{
  return(m_auxFrmCntr);
}

uint8_t
LrWpanMacHeader::GetSecLevel (void) const
{
  return (m_secctrlSecLevel);
}

uint8_t
LrWpanMacHeader::GetKeyIdMode (void) const
{
  return(m_secctrlKeyIdMode);
}

uint8_t
LrWpanMacHeader::GetSecCtrlReserved (void) const
{
  return (m_secctrlReserved);
}

uint32_t
LrWpanMacHeader::GetKeyIdSrc32 (void) const
{
  return(m_auxKeyIdKeySrc32);
}

uint64_t
LrWpanMacHeader::GetKeyIdSrc64 (void) const
{

  return(m_auxKeyIdKeySrc64);
}

uint8_t
LrWpanMacHeader::GetKeyIdIndex (void) const
{
  return(m_auxKeyIdKeyIndex);
}


bool
LrWpanMacHeader::IsBeacon (void) const
{
  return(m_fctrlFrmType == LRWPAN_MAC_BEACON);
}



bool
LrWpanMacHeader::IsData (void) const
{
  return(m_fctrlFrmType == LRWPAN_MAC_DATA);
}



bool
LrWpanMacHeader::IsAcknowledgment (void) const
{
  return(m_fctrlFrmType == LRWPAN_MAC_ACKNOWLEDGMENT);
}



bool
LrWpanMacHeader::IsCommand (void) const
{
  return(m_fctrlFrmType == LRWPAN_MAC_COMMAND);
}



void
LrWpanMacHeader::SetType (enum LrWpanMacType wpanMacType)
{
  m_fctrlFrmType = wpanMacType;
}


void
LrWpanMacHeader::SetFrameControl (uint16_t frameControl)
{
  m_fctrlFrmType = (frameControl) & (0x07);             // Bit 0-2
  m_fctrlSecU = (frameControl >> 3) & (0x01);           // Bit 3
  m_fctrlFrmPending = (frameControl >> 4) & (0x01);     // Bit 4
  m_fctrlAckReq = (frameControl >> 5) & (0x01);         // Bit 5
  m_fctrlPanIdComp = (frameControl >> 6) & (0x01);      // Bit 6



  m_fctrlDstAddrMode = (frameControl >> 10) & (0x03);   // Bit 10-11
  m_fctrlFrmVer = (frameControl >> 12) & (0x03);        // Bit 12-13
  m_fctrlSrcAddrMode = (frameControl >> 14) & (0x03);   // Bit 14-15


  if (m_fctrlFrmVer == 2)
    {
      //802.15.4e
      m_fctrlReserved = (frameControl >> 7) & (0x01);          // Bit 7
      m_fctrlSeqNumSuppression = (frameControl >> 8) & (0x01);          // Bit 8
      m_fctrlIEListPresent = (frameControl >> 9) & (0x01);          // Bit 9
    } else {
      m_fctrlReserved = (frameControl >> 7) & (0x07);             // Bit 7-9
    }
}


void
LrWpanMacHeader::SetSecEnable (void)
{
  m_fctrlSecU = 1;
}


void
LrWpanMacHeader::SetSecDisable (void)
{
  m_fctrlSecU = 0;
}


void
LrWpanMacHeader::SetFrmPend (void)
{
  m_fctrlFrmPending = 1;
}


void
LrWpanMacHeader::SetNoFrmPend (void)
{
  m_fctrlFrmPending = 0;
}


void
LrWpanMacHeader::SetAckReq (void)
{
  m_fctrlAckReq = 1;
}


void
LrWpanMacHeader::SetNoAckReq (void)
{
  m_fctrlAckReq = 0;
}


void
LrWpanMacHeader::SetPanIdComp (void)
{
  m_fctrlPanIdComp = 1;
}


void LrWpanMacHeader::SetNoPanIdComp (void)
{
  m_fctrlPanIdComp = 0;
}

void
LrWpanMacHeader::SetFrmCtrlRes (uint8_t res)
{
  m_fctrlReserved = res;
}

void
LrWpanMacHeader::SetDstAddrMode (uint8_t addrMode)
{
  m_fctrlDstAddrMode = addrMode;
}


void
LrWpanMacHeader::SetFrameVer (uint8_t ver)
{
  m_fctrlFrmVer = ver;
}


void
LrWpanMacHeader::SetSrcAddrMode (uint8_t addrMode)
{
  m_fctrlSrcAddrMode = addrMode;
}


void
LrWpanMacHeader::SetSeqNum (uint8_t seqNum)
{
  m_SeqNum = seqNum;
}

void
LrWpanMacHeader::SetSrcAddrFields (uint16_t panId,
                                   Mac16Address addr)
{
  m_addrSrcPanId = panId;
  m_addrShortSrcAddr = addr;
}

void
LrWpanMacHeader::SetSrcAddrFields (uint16_t panId,
                                   Mac64Address addr)
{
  m_addrSrcPanId = panId;
  m_addrExtSrcAddr = addr;
}

void
LrWpanMacHeader::SetDstAddrFields (uint16_t panId,
                                   Mac16Address addr)
{
  m_addrDstPanId = panId;
  m_addrShortDstAddr = addr;
}
void
LrWpanMacHeader::SetDstAddrFields (uint16_t panId,
                                   Mac64Address addr)
{
  m_addrDstPanId = panId;
  m_addrExtDstAddr = addr;
}
void
LrWpanMacHeader::SetSecControl (uint8_t secControl)
{
  m_secctrlSecLevel = (secControl) & (0x07);            // Bit 0-2
  m_secctrlKeyIdMode = (secControl >> 3) & (0x03);      // Bit 3-4
  m_secctrlReserved = (secControl >> 5) & (0x07);       // Bit 5-7
}

void
LrWpanMacHeader::SetFrmCounter (uint32_t frmCntr)
{
  m_auxFrmCntr = frmCntr;
}

void
LrWpanMacHeader::SetSecLevel (uint8_t secLevel)
{
  m_secctrlSecLevel = secLevel;
}

void
LrWpanMacHeader::SetKeyIdMode (uint8_t keyIdMode)
{
  m_secctrlKeyIdMode = keyIdMode;
}

void
LrWpanMacHeader::SetSecCtrlReserved (uint8_t res)
{
  m_secctrlReserved = res;
}

void
LrWpanMacHeader::SetKeyId (uint8_t keyIndex)
{
  m_auxKeyIdKeyIndex = keyIndex;
}


void
LrWpanMacHeader::SetKeyId (uint32_t keySrc,
                           uint8_t keyIndex)
{
  m_auxKeyIdKeyIndex = keyIndex;
  m_auxKeyIdKeySrc32 = keySrc;
}


void
LrWpanMacHeader::SetKeyId (uint64_t keySrc,
                           uint8_t keyIndex)
{
  m_auxKeyIdKeyIndex = keyIndex;
  m_auxKeyIdKeySrc64 = keySrc;
}


std::string
LrWpanMacHeader::GetName (void) const
{
  return "LrWpan MAC Header";
}

TypeId
LrWpanMacHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LrWpanMacHeader")
    .SetParent<Header> ()
    .AddConstructor<LrWpanMacHeader> ();
  return tid;
}


TypeId
LrWpanMacHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
LrWpanMacHeader::PrintFrameControl (std::ostream &os) const
{
  os << "Frame Type = " << (uint32_t) m_fctrlFrmType << ", Sec Enable = " << (uint32_t) m_fctrlSecU
     << ", Frame Pending = " << (uint32_t) m_fctrlFrmPending << ", Ack Request = " << (uint32_t) m_fctrlAckReq
     << ", PAN ID Compress = " << (uint32_t) m_fctrlPanIdComp;

  if (m_fctrlFrmVer == 2)
    {
      os << ", SeqNum Supression = "  << (uint32_t) m_fctrlSeqNumSuppression <<  ", IE List Present = " <<  (uint32_t) m_fctrlIEListPresent;
    }

  os << ", Dst Addrs Mode = " << (uint32_t) m_fctrlDstAddrMode
     << ", Frame Vers = " << (uint32_t) m_fctrlFrmVer << ", Src Addr Mode = " << (uint32_t) m_fctrlSrcAddrMode;
}

void
LrWpanMacHeader::Print (std::ostream &os) const
{
  PrintFrameControl (os);
  //os << std::endl;
  if (m_fctrlFrmVer != 2 || m_fctrlSeqNumSuppression == 0) {
    os << ", Sequence Num = " << static_cast<uint16_t> (m_SeqNum);
  }

  //802.15.4
  if (m_fctrlFrmVer == 2)
    {
    if (m_fctrlPanIdComp == 0)
      {
        if (m_fctrlDstAddrMode == NOADDR)
          {
            if (m_fctrlSrcAddrMode != NOADDR)
              {
                os << ", Src Addr Pan ID = " << static_cast<uint16_t> (m_addrSrcPanId);

                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    os << ", m_addrShortSrcAddr = " << m_addrShortSrcAddr;
                  }
                else
                  {
                    os << ", m_addrExtSrcAddr = " << m_addrExtDstAddr;
                  }
              }
          }
        else
          {
            os << ", Dst Addr Pan ID = " << static_cast<uint16_t> (m_addrDstPanId);

            if (m_fctrlDstAddrMode == SHORTADDR)
              {
                os << ", m_addrShortDstAddr = " << m_addrShortDstAddr;
              }
            else
              {
                os << ", m_addrExtDstAddr = " << m_addrExtDstAddr;
              }

            if (m_fctrlSrcAddrMode != NOADDR)
              {
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    os << ", m_addrShortSrcAddr = " << m_addrShortSrcAddr;
                  }
                else
                  {
                    os << ", m_addrExtSrcAddr = " << m_addrExtDstAddr;
                  }
              }
          }
        }
      else
        {
        if (m_fctrlDstAddrMode == NOADDR)
          {
            if (m_fctrlSrcAddrMode != NOADDR)
              {
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    os << ", m_addrShortSrcAddr = " << m_addrShortSrcAddr;
                  }
                else
                  {
                    os << ", m_addrExtSrcAddr = " << m_addrExtDstAddr;
                  }
              }
            else
              {
                 os << ", Dst Addr Pan ID = " << static_cast<uint16_t> (m_addrDstPanId);
              }
          }
        else
          {
            if (m_fctrlDstAddrMode == SHORTADDR)
              {
                os << ", m_addrShortSrcAddr = " << m_addrShortSrcAddr;
              }
            else
              {
                os << ", m_addrExtDstAddr = " << m_addrExtDstAddr;
              }

            if (m_fctrlSrcAddrMode != NOADDR)
              {
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    os << ", m_addrShortSrcAddr = " << m_addrShortSrcAddr;
                  }
                else
                  {
                    os << ", m_addrExtSrcAddr = " << m_addrExtDstAddr;
                  }
              }
          }
      }
    }
  else
    {
      switch (m_fctrlDstAddrMode)
        {
        case NOADDR:
          break;
        case SHORTADDR:
          os << ", Dst Addr Pan ID = " << static_cast<uint16_t> (m_addrDstPanId)
             << ", m_addrShortDstAddr = " << m_addrShortDstAddr;
          break;
        case EXTADDR:
          os << ", Dst Addr Pan ID = " << static_cast<uint16_t> (m_addrDstPanId)
             << ", m_addrExtDstAddr = " << m_addrExtDstAddr;
          break;
        }

      switch (m_fctrlSrcAddrMode)
        {
        case NOADDR:
          break;
        case SHORTADDR:
          os << ", Src Addr Pan ID = " << static_cast<uint16_t> (m_addrSrcPanId)
             << ", m_addrShortSrcAddr = " << m_addrShortSrcAddr;
          break;
        case EXTADDR:
          os << ", Src Addr Pan ID = " << static_cast<uint32_t> (m_addrSrcPanId)
             << ", m_addrExtSrcAddr = " << m_addrExtDstAddr;
          break;
        }
    }

  if (IsSecEnable ())
    {
      os << "  Security Level = " << static_cast<uint32_t> (m_secctrlSecLevel)
         << ", Key Id Mode = " << static_cast<uint32_t> (m_secctrlKeyIdMode)
         << ", Frame Counter = " << static_cast<uint32_t> (m_auxFrmCntr);

      switch (m_secctrlKeyIdMode)
        {
        case IMPLICIT:
          break;
        case NOKEYSOURCE:
          os << ", Key Id - Key Index = " << static_cast<uint32_t> (m_auxKeyIdKeyIndex);
          break;
        case SHORTKEYSOURCE:
          os << ", Key Id - Key Source 32 =" << static_cast<uint32_t> (m_auxKeyIdKeySrc32)
             << ", Key Id - Key Index = " << static_cast<uint32_t> (m_auxKeyIdKeyIndex);
          break;
        case LONGKEYSOURCE:
          os << ", Key Id - Key Source 64 =" << static_cast<uint64_t> (m_auxKeyIdKeySrc64)
             << ", Key Id - Key Index = " << static_cast<uint32_t> (m_auxKeyIdKeyIndex);
          break;
        }
    }

  if (m_fctrlIEListPresent == 1) {
    os << " IE List: ";

    for (std::list<HeaderIE>::const_iterator it = headerie.begin();it!=headerie.end();it++) {

      os << "Lenght = " << (uint32_t) it->length
         << ", Type = " << (uint32_t) it->type
         << ", ID = " << (uint32_t)  it->id;

      os << ", Data: ";
      for (std::vector<uint8_t>::const_iterator it2 = it->content.begin();it2!=it->content.end();it2++) {
        os << static_cast<uint32_t>(*it2);
      }
      os << "; ";
      
    }
  }
}

uint32_t
LrWpanMacHeader::GetSerializedSize (void) const
{
  /*
   * Each mac header will have
   * Frame Control      : 2 octet
   * Sequence Number    : 0/1 Octet
   * Dst PAN Id         : 0/2 Octet
   * Dst Address        : 0/2/8 octet
   * Src PAN Id         : 0/2 octet
   * Src Address        : 0/2/8 octet
   * Aux Sec Header     : 0/5/6/10/14 octet
   * IE Header          : variable
   */

  uint32_t size = 2;

  if ((m_fctrlSeqNumSuppression == 0 && m_fctrlFrmVer == 2) || m_fctrlFrmVer == 1)
    {
      size+=1;
    }

  if (m_fctrlFrmVer == 2)
    {
    if (m_fctrlPanIdComp == 0)
      {
        if (m_fctrlDstAddrMode == NOADDR)
          {
            if (m_fctrlSrcAddrMode != NOADDR)
              {
                size+=2;
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    size+=2;
                  }
                else
                  {
                    size+=4;
                  }
              }
          }
        else
          {
            size+=2;

            if (m_fctrlDstAddrMode == SHORTADDR)
              {
                size+=2;
              }
            else
              {
               size+=4;
              }

            if (m_fctrlSrcAddrMode != NOADDR)
              {
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    size+=2;
                  }
                else
                  {
                    size+=4;
                  }
              }
          }
        }
      else
        {
        if (m_fctrlDstAddrMode == NOADDR)
          {
            if (m_fctrlSrcAddrMode != NOADDR)
              {
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    size+=2;
                  }
                else
                  {
                    size+=4;
                  }
              }
            else
              {
                 size+=2;
              }
          }
        else
          {
            if (m_fctrlDstAddrMode == SHORTADDR)
              {
                size+=2;
              }
            else
              {
                size+=4;
              }

            if (m_fctrlSrcAddrMode != NOADDR)
              {
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    size+=2;
                  }
                else
                  {
                    size+=4;
                  }
              }
          }
      }
    }
  else
    {
      switch (m_fctrlDstAddrMode)
        {
        case NOADDR:
          break;
        case SHORTADDR:
          size += 4;
          break;
        case EXTADDR:
          size += 10;
          break;
        }

      switch (m_fctrlSrcAddrMode)
        {
        case NOADDR:
          break;
        case SHORTADDR:
          // check if PAN Id compression is enabled
          if (!IsPanIdComp ())
            {
              size += 4;
            }
          else
            {
              size += 2;
            }
          break;
        case EXTADDR:
          // check if PAN Id compression is enabled
          if (!IsPanIdComp ())
            {
              size += 10;
            }
          else
            {
              size += 8;
            }
          break;
        }
    }

  // check if security is enabled
  if (IsSecEnable ())
    {
      size += 5;
      switch (m_secctrlKeyIdMode)
        {
        case IMPLICIT:
          break;
        case NOKEYSOURCE:
          size += 1;
          break;
        case SHORTKEYSOURCE:
          size += 5;
          break;
        case LONGKEYSOURCE:
          size += 9;
          break;
        }
    }

  if (m_fctrlIEListPresent == 1 && m_fctrlFrmVer == 2) {
    for (std::list<HeaderIE>::const_iterator it = headerie.begin() ; it != headerie.end() ; it++) {
      size+=it->length + 2;
    }
  }

  return (size);
}


void
LrWpanMacHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  uint16_t frameControl = GetFrameControl ();

  i.WriteHtolsbU16 (frameControl);

  if ((m_fctrlSeqNumSuppression == 0 || m_fctrlFrmVer!=2) || m_fctrlFrmVer == 1) {
    i.WriteU8 (GetSeqNum ());
  }

  //802.15.4
  if (m_fctrlFrmVer == 2)
    {
    if (m_fctrlPanIdComp == 0)
      {
        if (m_fctrlDstAddrMode == NOADDR)
          {
            if (m_fctrlSrcAddrMode != NOADDR)
              {
                i.WriteHtolsbU16 (GetSrcPanId ());
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    WriteTo (i, m_addrShortSrcAddr);
                  }
                else
                  {
                    WriteTo (i, m_addrExtSrcAddr);
                  }
              }
          }
        else
          {
            i.WriteHtolsbU16 (GetDstPanId ());

            if (m_fctrlDstAddrMode == SHORTADDR)
              {
                WriteTo (i, m_addrShortDstAddr);
              }
            else
              {
                WriteTo (i, m_addrExtDstAddr);
              }

            if (m_fctrlSrcAddrMode != NOADDR)
              {
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    WriteTo (i, m_addrShortSrcAddr);
                  }
                else
                  {
                    WriteTo (i, m_addrExtSrcAddr);
                  }
              }
          }
        }
      else
        {
        if (m_fctrlDstAddrMode == NOADDR)
          {
            if (m_fctrlSrcAddrMode != NOADDR)
              {
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    WriteTo (i, m_addrShortSrcAddr);
                  }
                else
                  {
                    WriteTo (i, m_addrExtSrcAddr);
                  }
              }
            else
              {
                 i.WriteHtolsbU16 (GetDstPanId ());
              }
          }
        else
          {
            if (m_fctrlDstAddrMode == SHORTADDR)
              {
                WriteTo (i, m_addrShortDstAddr);
              }
            else
              {
                WriteTo (i, m_addrExtDstAddr);
              }

            if (m_fctrlSrcAddrMode != NOADDR)
              {
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    WriteTo (i, m_addrShortSrcAddr);
                  }
                else
                  {
                    WriteTo (i, m_addrExtSrcAddr);
                  }
              }
          }
      }
    }
  else
    {
      switch (m_fctrlDstAddrMode)
        {
          case NOADDR:
            break;
          case SHORTADDR:
            i.WriteHtolsbU16 (GetDstPanId ());
            WriteTo (i, m_addrShortDstAddr);
            break;
          case EXTADDR:
            i.WriteHtolsbU16 (GetDstPanId ());
            WriteTo (i, m_addrExtDstAddr);
            break;
        }

      switch (m_fctrlSrcAddrMode)
        {
          case NOADDR:
            break;
          case SHORTADDR:
            if (!IsPanIdComp ())
              {
                i.WriteHtolsbU16 (GetSrcPanId ());
              }
            WriteTo (i, m_addrShortSrcAddr);
            break;
          case EXTADDR:
            if (!IsPanIdComp ())
              {
                i.WriteHtolsbU16 (GetSrcPanId ());
              }
            WriteTo (i, m_addrExtSrcAddr);
            break;
        }
    }

  if (IsSecEnable ())
    {
      i.WriteU8 (GetSecControl ());
      i.WriteHtolsbU32 (GetFrmCounter ());

      switch (m_secctrlKeyIdMode)
        {
        case IMPLICIT:
          break;
        case NOKEYSOURCE:
          i.WriteU8 (GetKeyIdIndex ());
          break;
        case SHORTKEYSOURCE:
          i.WriteHtolsbU32 (GetKeyIdSrc32 ());
          i.WriteU8 (GetKeyIdIndex ());
          break;
        case LONGKEYSOURCE:
          i.WriteHtolsbU64 (GetKeyIdSrc64 ());
          i.WriteU8 (GetKeyIdIndex ());
          break;
        }
    }

  if (m_fctrlIEListPresent == 1 && m_fctrlFrmVer == 2) {
    for (std::list<HeaderIE>::const_iterator it = headerie.begin();it!=headerie.end();it++) {
      uint16_t desc = 0;

      desc |= (it->length << 9) & (0xfe00); //7bits
      //desc |= 0x00 ; //1 bit
      desc |= (it->id << 1) & (0x01fe); //8bits
          
      i.WriteHtolsbU16(desc);

      for (uint8_t j = 0; j < it->length ;j++) {
        i.WriteU8 (it->content[j]);
      }
    }
  }
}


uint32_t
LrWpanMacHeader::Deserialize (Buffer::Iterator start)
{

  Buffer::Iterator i = start;
  uint16_t frameControl = i.ReadLsbtohU16 ();
  SetFrameControl (frameControl);

  if (m_fctrlSeqNumSuppression == 0 || m_fctrlFrmVer == 1) {
    SetSeqNum (i.ReadU8 ());
  }

  //802.15.4
  if (m_fctrlFrmVer == 2)
    {
    if (m_fctrlPanIdComp == 0)
      {
        if (m_fctrlDstAddrMode == NOADDR)
          {
            if (m_fctrlSrcAddrMode != NOADDR)
              {
                m_addrSrcPanId = i.ReadLsbtohU16 ();
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    ReadFrom (i, m_addrShortSrcAddr);
                  }
                else
                  {
                    ReadFrom (i, m_addrExtSrcAddr);
                  }
              }
          }
        else
          {
            m_addrDstPanId = i.ReadLsbtohU16 ();

            if (m_fctrlDstAddrMode == SHORTADDR)
              {
                ReadFrom (i, m_addrShortDstAddr);
              }
            else
              {
                ReadFrom (i, m_addrExtDstAddr);
              }

            if (m_fctrlSrcAddrMode != NOADDR)
              {
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    ReadFrom (i, m_addrShortSrcAddr);
                  }
                else
                  {
                    ReadFrom (i, m_addrExtSrcAddr);
                  }
              }
          }
        }
      else
        {
        if (m_fctrlDstAddrMode == NOADDR)
          {
            if (m_fctrlSrcAddrMode != NOADDR)
              {
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    ReadFrom (i, m_addrShortSrcAddr);
                  }
                else
                  {
                    ReadFrom (i, m_addrExtSrcAddr);
                  }
              }
            else
              {
                 m_addrDstPanId = i.ReadLsbtohU16 ();
              }
          }
        else
          {
            if (m_fctrlDstAddrMode == SHORTADDR)
              {
                ReadFrom (i, m_addrShortDstAddr);
              }
            else
              {
                ReadFrom (i, m_addrExtDstAddr);
              }

            if (m_fctrlSrcAddrMode != NOADDR)
              {
                if (m_fctrlSrcAddrMode == SHORTADDR)
                  {
                    ReadFrom (i, m_addrShortSrcAddr);
                  }
                else
                  {
                    ReadFrom (i, m_addrExtSrcAddr);
                  }
              }
          }
      }
    }
  else
    {
      switch (m_fctrlDstAddrMode)
        {
        case NOADDR:
          break;
        case SHORTADDR:
          m_addrDstPanId = i.ReadLsbtohU16 ();
          ReadFrom (i, m_addrShortDstAddr);
          break;
        case EXTADDR:
          m_addrDstPanId = i.ReadLsbtohU16 ();
          ReadFrom (i, m_addrExtDstAddr);
          break;
        }

      switch (m_fctrlSrcAddrMode)
        {
        case NOADDR:
          break;
        case SHORTADDR:
          if (!IsPanIdComp ())
            {
              m_addrSrcPanId = i.ReadLsbtohU16 ();
            }
          else
            {
              if (m_fctrlDstAddrMode > 0)
                {
                  m_addrSrcPanId = m_addrDstPanId;
                }
            }
          ReadFrom (i, m_addrShortSrcAddr);
          break;
        case EXTADDR:
          if (!IsPanIdComp ())
            {
              m_addrSrcPanId = i.ReadLsbtohU16 ();
            }
          else
            {
              if (m_fctrlDstAddrMode > 0)
                {
                  m_addrSrcPanId = m_addrDstPanId;
                }
            }
          ReadFrom (i, m_addrExtSrcAddr);
          break;
        }
    }

  if (IsSecEnable ())
    {
      SetSecControl (i.ReadU8 ());
      SetFrmCounter (i.ReadLsbtohU32 ());

      switch (m_secctrlKeyIdMode)
        {
        case IMPLICIT:
          break;
        case NOKEYSOURCE:
          SetKeyId (i.ReadU8 ());
          break;
        case SHORTKEYSOURCE:
          SetKeyId (i.ReadLsbtohU32 (),i.ReadU8 ());
          break;
        case LONGKEYSOURCE:
          SetKeyId (i.ReadLsbtohU64 (),i.ReadU8 ());
          break;
        }
    }

    //802.15.4 IE Header
    if (m_fctrlFrmVer == 2 && m_fctrlIEListPresent == 1) {
      uint8_t lastid;

      do {
        HeaderIE* newie = new HeaderIE;
        uint16_t head = i.ReadLsbtohU16 ();
        newie->length = (head >> 9); //7 bits
        newie->id = (head >> 1); //8bits
        newie->type = 0; //1bit

        for (int j = 0;j<newie->length ;j++) {
          newie->content.push_back(i.ReadU8 ());
        }

        headerie.push_back(*newie);
        lastid = newie->id;

      } while (lastid != 0x7e && lastid != 0x7f);
    }

  return i.GetDistanceFrom (start);
}


//802.15.4e
void 
LrWpanMacHeader::NewAckIE(uint16_t t)
{
  HeaderIE ack;

  ack.length = 2;
  ack.id = 0x1e;
  ack.type = 0x0;

  ack.content.push_back((t >> (8)) & 0xff);
  ack.content.push_back(t);

  headerie.push_back(ack);
}

void
LrWpanMacHeader::EndNoPayloadIE()
{

  if (m_fctrlFrmVer == 2) {
    if (m_fctrlIEListPresent == 1) {
      HeaderIE endie;
      endie.id = 0x7f;
      endie.length = 0;

      headerie.push_back(endie);
    }
  }
}

void
LrWpanMacHeader::EndPayloadIE() 
{

  if (m_fctrlFrmVer == 2) {
    if (m_fctrlIEListPresent == 1) {
      HeaderIE endie;
      endie.id = 0x7e;
      endie.length = 0;

      headerie.push_back(endie);
    }
  }
}

std::list<LrWpanMacHeader::HeaderIE>
LrWpanMacHeader::GetIEList(void) const
{
  return headerie;
}

bool
LrWpanMacHeader::IsSeqNumSup (void) const
{
  return (m_fctrlSeqNumSuppression == 1);
}

bool
LrWpanMacHeader::IsNoSeqNumSup (void) const
{
  return (m_fctrlSeqNumSuppression == 0);
}

void LrWpanMacHeader::SetIEField()
{
  m_fctrlIEListPresent = 1;
}

void LrWpanMacHeader::SetNoIEField()
{
  m_fctrlIEListPresent = 0;
}

void LrWpanMacHeader::SetSeqNumSup()
{
  m_fctrlSeqNumSuppression = 1;
}

void LrWpanMacHeader::SetNoSeqNumSup()
{
  m_fctrlSeqNumSuppression = 0;
}

bool LrWpanMacHeader::IsIEListPresent (void) const
{
  return (m_fctrlIEListPresent == 1);
}
// ----------------------------------------------------------------------------------------------------------


} //namespace ns3


