#pragma once

#include <stdint.h>

namespace UartDriver
{

  static constexpr uint8_t ESC = 0x1B;  // the escape symbol
  static constexpr uint8_t SOH = 0x01;  // start of header
  static constexpr uint8_t STX = 0x02;  // start of text (regular message)

  static constexpr unsigned MAX_CONTENT_SIZE = 256;

  typedef void (*UartReceiveComplete_Callback)(uint8_t const contentId, uint8_t const len, uint8_t const* const pData);

  class RxParser
  {
   public:
    RxParser(UartReceiveComplete_Callback receiveCallback)
        : m_receiveCallback(receiveCallback)
    {
      reset();
    };

   private:
    UartReceiveComplete_Callback m_receiveCallback;

    enum class States
    {
      WaitForSync1stByte,
      WaitForSync2ndByte,
      WaitForSync3rdByte,
      WaitForSync4thByte,
      CollectContent,
    };

    int16_t     m_lastByte;
    enum States m_state;
    uint8_t     m_contentId;
    uint8_t     m_contentSize;
    uint8_t     m_contentBytesRemaining;
    uint16_t    m_contentIndex;
    uint8_t     m_content[MAX_CONTENT_SIZE];

    inline void reset(void)
    {
      m_state    = States::WaitForSync1stByte;
      m_lastByte = -1;
    };

    enum class RxParserErrors
    {
      State,
      Sync,
      TypeId,
      ContentId,
      CollectData,
    };

    inline void reset(enum RxParserErrors const error)
    {
      reset();
    }

    enum class CollectStates
    {
      WaitForId,
      WaitForSize,
      CollectContent
    };

    enum CollectStates m_collectState;

    inline void startCollectContent(void)
    {
      m_collectState = CollectStates::WaitForId;
      m_lastByte     = -1;
    };

    enum class CollectResult
    {
      Collecting,
      Done,
      Error,
    };

    enum CollectResult collectContent(uint8_t const byte)
    {
      switch (m_collectState)
      {
        case CollectStates::WaitForId:
          m_contentId    = byte;
          m_collectState = CollectStates::WaitForSize;
          return CollectResult::Collecting;

        case CollectStates::WaitForSize:
          m_contentSize = m_contentBytesRemaining = byte;
          if (m_contentSize == 0)
            break;
          m_collectState = CollectStates::CollectContent;
          return CollectResult::Collecting;

        case CollectStates::CollectContent:
          if (m_lastByte == ESC && byte != ESC)
            return CollectResult::Error;

          if (!(m_lastByte == ESC && byte == ESC))
          {
            m_content[m_contentIndex++] = byte;
            m_contentBytesRemaining--;
          }
          m_lastByte = byte;
          break;
      }

      if (m_contentBytesRemaining == 0)
      {
        (*m_receiveCallback)(m_contentId, m_contentSize, &m_content[0]);
        return CollectResult::Done;
      }
      return CollectResult::Collecting;
    };

   public:
    // parses incoming raw bytes from the UART, called from driver
    // once a message is assembled and decoded, the content handlers are called
    void onByteReceived(uint8_t const byte)
    {
      switch (m_state)
      {
        default:
          reset(RxParserErrors::State);
          break;

        case States::WaitForSync1stByte:
          if (byte == ESC)
            m_state = States::WaitForSync2ndByte;
          break;

        case States::WaitForSync2ndByte:
          if (byte == SOH)
            m_state = States::WaitForSync3rdByte;
          else
            reset(RxParserErrors::Sync);
          break;

        case States::WaitForSync3rdByte:
          if (byte == ESC)
            m_state = States::WaitForSync4thByte;
          else
            reset(RxParserErrors::TypeId);
          break;

        case States::WaitForSync4thByte:
          if (byte == STX)
          {
            startCollectContent();
            m_state = States::CollectContent;
          }
          else
            reset(RxParserErrors::TypeId);
          break;

        case States::CollectContent:
          switch (collectContent(byte))
          {
            case CollectResult::Collecting:
              break;

            case CollectResult::Done:
              reset();
              break;

            case CollectResult::Error:
              reset(RxParserErrors::CollectData);
              break;
          }
          break;
      }
    };

  };  //class

}  // namespace
