// ------- UART (Serial Interface) Driver, middle layer -------

/*
 * The driver parses the incoming bytes from the hardware driver and calls the application layer callback
 * once a message is complete.
 *
 */

#pragma once

#include <stdint.h>
#include "uartProtocolDefs.h"

namespace UartProtocol
{
  static constexpr unsigned MAX_CONTENT_SIZE = 255;

  typedef void (*UartReceiveComplete_Callback)(uint8_t contentId, uint8_t len, uint8_t *pData);
  typedef int (*UartTransmit_Callback)(uint8_t data, uint32_t const lineStatus);

  // ---------------- // ----------------

  class RxParser
  {
   public:
    RxParser(UartReceiveComplete_Callback receiveCallback)
        : m_receiveCallback(receiveCallback)
    {
      reset();
    };

    // ----------------

    enum class Errors  // error specifiers
    {
      None,
      Hardware,     // UART hardware/protocol errors like framing error, break, parity error etc
      Sync,         // error in header/magic
      MessageID,    // Message ID must not be ESC
      PayloadSize,  // Payload Size must not be ESC
      PayloadData,  // Payload Data must not contain single ESC's
      ParserState,  // illegal state machine state
    };

    // ----------------

    // Parses incoming raw bytes from the UART, called from the driver.
    // Negative input values are used by the hardware driver to flag hardware errors.
    // Once a message is assembled and decoded, the content handler callback is called
    inline void onByteReceived(int16_t const byte)
    {
      if (byte < 0)
      {
        reset(Errors::Hardware);
        return;
      }

      switch (m_parserState)
      {
        default:
          reset(Errors::ParserState);
          break;

        case ParserStates::WaitForSync1stByte:
          if (byte == MAGIC0)
            m_parserState = ParserStates::WaitForSync2ndByte;
          break;

        case ParserStates::WaitForSync2ndByte:
          if (byte == MAGIC1)
            m_parserState = ParserStates::WaitForSync3rdByte;
          else
            reset(Errors::Sync);
          break;

        case ParserStates::WaitForSync3rdByte:
          if (byte == MAGIC2)
            m_parserState = ParserStates::WaitForSync4thByte;
          else
            reset(Errors::Sync);
          break;

        case ParserStates::WaitForSync4thByte:
          if (byte == MAGIC3)
          {
            startCollectContent();
            m_parserState = ParserStates::CollectPayload;
          }
          else
            reset(Errors::Sync);
          break;

        case ParserStates::CollectPayload:
          collectContent(uint8_t(byte));
          break;
      }  // switch (m_state)
    };

    inline enum Errors getAndClearError(void)
    {
      Errors retVal = m_error;
      m_error       = Errors::None;
      return retVal;
    };

    inline void reset(void)
    {
      m_parserState = ParserStates::WaitForSync1stByte;
      m_error       = Errors::None;
    };

    // ---------------- // ----------------

   private:
    enum class ParserStates  // of the parser state machine
    {
      WaitForSync1stByte,
      WaitForSync2ndByte,
      WaitForSync3rdByte,
      WaitForSync4thByte,
      CollectPayload,
    };

    enum class CollectStates
    {
      WaitForId,
      WaitForSize,
      CollectPayload,
    };

    // ----------------

    enum ParserStates            m_parserState;
    bool                         m_hadEsc;
    uint8_t                      m_messageId;
    uint8_t                      m_payloadSize;
    uint8_t                      m_payloadBytesRemaining;
    uint8_t                      m_payloadArrayIndex;
    uint8_t                      m_payloadArray[MAX_CONTENT_SIZE];
    enum CollectStates           m_collectState;
    enum Errors                  m_error;
    UartReceiveComplete_Callback m_receiveCallback;

    // ----------------

    inline void reset(enum Errors const error)
    {
      reset();
      m_error = error;
    }

    inline void startCollectContent(void)
    {
      m_collectState = CollectStates::WaitForId;
    };

    inline void collectContent(uint8_t byte)
    {
      switch (m_collectState)
      {
        case CollectStates::WaitForId:
          if (byte == ESC)
          {
            reset(Errors::MessageID);
            return;
          }
          m_messageId    = byte;
          m_collectState = CollectStates::WaitForSize;
          return;

        case CollectStates::WaitForSize:
          if (byte == ESC)  // ESC is not allowed as value...
          {
            reset(Errors::PayloadSize);
            return;
          }
          if (byte > ESC)
            --byte;  // ...and forms a "hole" in the value space for sizes
          m_payloadSize = m_payloadBytesRemaining = byte;

          if (m_payloadSize == 0)  // size == 0 --> no data --> finished
            break;

          m_collectState      = CollectStates::CollectPayload;
          m_hadEsc            = false;
          m_payloadArrayIndex = 0;
          return;

        case CollectStates::CollectPayload:
        {
          bool update = true;

          if (m_hadEsc)
          {
            if (byte != ESC)
            {
              reset(Errors::PayloadData);
              return;
            }
          }
          else
          {
            if (byte == ESC)
            {
              m_hadEsc = true;
              update   = false;
            }
          }

          if (update)
          {
            m_payloadArray[m_payloadArrayIndex++] = byte;
            m_payloadBytesRemaining--;
            m_hadEsc = false;
          }
          break;
        }
      }  // switch (m_collectState)

      if (m_payloadBytesRemaining == 0)
      {
        (*m_receiveCallback)(m_messageId, m_payloadSize, &m_payloadArray[0]);
        reset();
      }
    };

  };  // class RxParser

  // --------------------------- // --------------------------- // ---------------------------
  class TxAssembler
  {
   public:
    TxAssembler(UartTransmit_Callback transmitCallback)
        : m_transmitCallback(transmitCallback) {};

    inline bool processPendingTransmits(uint32_t const lineStatus)
    {
      if (m_head == m_tail)
        return false;

      if ((*m_transmitCallback)(m_txBuffer[m_tail], lineStatus))
        m_tail++;
      return true;
    };

    inline void putHeader(void)
    {
      putRawByte(MAGIC0);
      putRawByte(MAGIC1);
      putRawByte(MAGIC2);
      putRawByte(MAGIC3);
    };

    inline void putMessageId(MessageIds const id)
    {
      putRawByte(uint8_t(id));
    };

    inline void putTan(uint16_t const tan)
    {
      putPayloadDataByte(uint8_t(tan >> 8));
      putPayloadDataByte(uint8_t(tan & 0xFF));
    };

    inline void putPayloadSize(uint8_t byte)
    {
      if (byte >= ESC)
        byte++;
      putRawByte(byte);
    };

    inline void putPayloadDataByte(uint8_t const byte)
    {
      putRawByte(byte);
      if (byte == ESC)
        putRawByte(byte);
    };

    inline void putRawByte(uint8_t const byte)
    {
      if (uint8_t(m_head + 1) != m_tail)
        m_txBuffer[m_head++] = byte;
      else
      {
        // TODO: handle buffer overrun
      }
    };

   private:
    UartTransmit_Callback m_transmitCallback;
    uint8_t               m_txBuffer[256];  // must be 256 to afford simple uint8_t modulo indexing
    uint8_t               m_head { 0 };
    uint8_t               m_tail { 0 };

  };  // class TxAssembler

}  // namespace UartProtocol
