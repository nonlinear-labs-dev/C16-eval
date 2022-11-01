// ------- UART (Serial Interface) Driver, ReceiveParser -------

/*
 * This is the application layer transmit assembler which creates and queue messages
 *
 */

#pragma once

#include <stdint.h>
#include "uartProtocolDefs.h"
#include "uartProtocol.h"
#include "drv/LRApatternIds.h"

namespace UartProtocol
{

  class MessageComposer
  {
   public:
    MessageComposer(UartProtocol::TxAssembler& txAssembler)
        : m_txAssembler(txAssembler) {};

    inline void sendAck(uint8_t const tanHi, uint8_t const tanLo) const
    {
      m_txAssembler.putHeader();
      m_txAssembler.putMessageId(MessageIds::Acknowledge);
      m_txAssembler.putPayloadSize(2);
      m_txAssembler.putRawByte(tanHi);
      m_txAssembler.putRawByte(tanLo);
    };

    inline void sendEhcSetup(uint8_t const seriesRs, uint8_t const pullupRs, uint8_t const pulldownRs)
    {
      m_txAssembler.putHeader();
      m_txAssembler.putMessageId(MessageIds::EhcSetup);
      m_txAssembler.putPayloadSize(2 + 3);
      m_txAssembler.putTan(m_tan++);
      m_txAssembler.putPayloadDataByte(seriesRs);
      m_txAssembler.putPayloadDataByte(pullupRs);
      m_txAssembler.putPayloadDataByte(pulldownRs);
    }

    inline void sendLraControl(uint8_t const lraNum, LRA::LraPatternsIds const patternId)
    {
      if (lraNum > 7)
        return;
      m_txAssembler.putHeader();
      m_txAssembler.putMessageId(MessageIds::LraControl);
      m_txAssembler.putPayloadSize(2 + 1);
      m_txAssembler.putTan(m_tan++);

      uint8_t lraCtrl = lraNum << 5;
      lraCtrl |= uint8_t(patternId);
      m_txAssembler.putPayloadDataByte(lraCtrl);
    };

   private:
    UartProtocol::TxAssembler& m_txAssembler;
    uint16_t                   m_tan { 0 };
  };

}  // namespace
