#pragma once

#include <drv/IoPin.h>
#include <stdint.h>
#include "usb/driver/nl_usb_midi.h"
#include "usb/driver/nl_usb_core_circular_buffers.h"
#include "tasks/mtask.h"

namespace Usb
{
  static inline uint8_t getSysexHi4Byte(unsigned const value)
  {
    return (uint8_t)((value & 0b11110000000000000000000000000000) >> 28);
  }

  static inline uint8_t getSysexHi3Byte(unsigned const value)
  {
    return (value & 0b1111111000000000000000000000) >> 21;
  }

  static inline uint8_t getSysexHi2Byte(unsigned const value)
  {
    return (value & 0b111111100000000000000) >> 14;
  }

  static inline uint8_t getSysexHiByte(unsigned const value)
  {
    return (value & 0b11111110000000) >> 7;
  }

  static inline uint8_t getSysexLoByte(unsigned const value)
  {
    return (value & 0b1111111);
  }

  // class with templated parameter is used for code efficiency.
  // a class with real parameter, or subclasses using overloading, resulted in much slower code
  template <enum USB_BufferType tBufferType>
  class UsbMidiSysexWriter
  {
   private:
    static constexpr unsigned BUFFER_ELEM_COUNT = unsigned(tBufferType) / sizeof(uint32_t);
    uint32_t *                m_buffer;
    unsigned                  m_bufIndex                    = 0;
    unsigned                  m_sendBufferIndex             = 0;
    unsigned                  m_currentTransactionElemCount = 0;
    IOpins::IOpin &           m_LED_usbStalling;

    // relies on buffer sizes being 2^N for efficiency (modulo operator optimized out)
    inline unsigned modBufferSize(unsigned const x) const
    {
      return x % BUFFER_ELEM_COUNT;
    };

    inline void advanceIndex(void)
    {
      m_bufIndex = modBufferSize(m_bufIndex + 1u);
    }

    inline unsigned usedBuffer(void) const
    {
      return modBufferSize(BUFFER_ELEM_COUNT + m_bufIndex - m_sendBufferIndex);
    };

   public:
    constexpr UsbMidiSysexWriter(uint32_t *const buffer, IOpins::IOpin &LED_usbStalling)
        : m_buffer(buffer)
        , m_LED_usbStalling(LED_usbStalling) {};

    inline int claimBufferElements(unsigned const requestedElemCount) const
    {
      unsigned freeCount = BUFFER_ELEM_COUNT - usedBuffer() - m_currentTransactionElemCount;
      return freeCount > requestedElemCount;
    };

    inline void write(uint8_t const cableNumber, uint8_t const byte1, uint8_t const byte2, uint8_t const byte3)
    {
      uint32_t packet      = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | ((cableNumber << 4) | 0x04);
      m_buffer[m_bufIndex] = packet;
      advanceIndex();
    };

    inline void writeLast(uint8_t const cableNumber, uint8_t const byte1, uint8_t const byte2, uint8_t const byte3)
    {
      uint32_t packet      = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | ((cableNumber << 4) | 0x07);
      m_buffer[m_bufIndex] = packet;
      advanceIndex();
    };

    inline void writeLast(uint8_t const cableNumber, uint8_t const byte1, uint8_t const byte2)
    {
      uint32_t packet      = (byte2 << 16) | (byte1 << 8) | ((cableNumber << 4) | 0x06);
      m_buffer[m_bufIndex] = packet;
      advanceIndex();
    };

    inline void writeLast(uint8_t const cableNumber, uint8_t const byte1)
    {
      uint32_t packet      = (byte1 << 8) | ((cableNumber << 4) | 0x05);
      m_buffer[m_bufIndex] = packet;
      advanceIndex();
    };

    inline void processTransaction(void)
    {
      if (USB_MIDI_BytesToSend(0) == 0)
        m_currentTransactionElemCount = 0;

      if (m_sendBufferIndex == m_bufIndex)
        return;

      uint8_t *sendBuffer    = (uint8_t *) (&m_buffer[m_sendBufferIndex]);
      unsigned sendBufferLen = usedBuffer() * sizeof m_buffer[0];
      if (USB_MIDI_Send(0, sendBuffer, sendBufferLen) == -1)
      {  // failed
        m_LED_usbStalling.timedOn(1);
        return;
      }
      m_sendBufferIndex             = m_bufIndex;
      m_currentTransactionElemCount = sendBufferLen / sizeof m_buffer[0];
    };
  };

}  // namespace
