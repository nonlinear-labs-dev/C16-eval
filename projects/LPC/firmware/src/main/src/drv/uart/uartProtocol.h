// ------- UART (Serial Interface) Driver, middle layer -------

/*
 * This driver parses the incoming bytes from the hardware driver and calls the application layer callback
 * once a message is complete.
 *
 * Every message consists of a header, formed by 4 magic bytes, and a a body, containing the message ID,
 * the message length and the actual message payload (which can be zero length).
 *
 * The header is also used to reliably sync receiver to the transmitter when communication sets in
 * at an arbitrary point (or breaks and resumes).
 *
 * This is achieved by the special Escape pattern that is never appearing outside
 * of the header section because
 *  - outside of the header every ESC symbol is doubled,
 *  - message ID cannot be ESC, that is, there is a hole in the value space for message ID, ranging from 0..254 only,
 *  - message size cannot be ESC. Values >= ESC are encoded as Value+1, again creating a hole in the value
 *    range, now spanning 0..254 only.
 *
 *
 * Logical Message Structure:
 *   ESC | SOH | ESC | STX | message-ID | payload-size (N) | [ payload byte 1 | ... | payload byte N ]
 *
 * In the physical message, every byte in the payload section that is equal to ESC has to be doubled,
 * for example | 19 | 1A | 1B | 1C | becomes | 19 | 1A | 1B | 1B | 1C |
 *
 */

#pragma once

#include <stdint.h>

namespace UartProtocol
{
  static constexpr uint8_t ESC = 0x1B;  // the escape symbol
  static constexpr uint8_t SOH = 0x01;  // start of header
  static constexpr uint8_t STX = 0x02;  // start of text

  static constexpr uint8_t MAGIC0 = ESC;
  static constexpr uint8_t MAGIC1 = SOH;
  static constexpr uint8_t MAGIC2 = ESC;
  static constexpr uint8_t MAGIC3 = STX;

  static constexpr unsigned MAX_CONTENT_SIZE = 255;

  typedef void (*UartReceiveComplete_Callback)(uint8_t contentId, uint8_t len, uint8_t *pData);

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

  };  //class RxParser

}  // namespace UartDriver
