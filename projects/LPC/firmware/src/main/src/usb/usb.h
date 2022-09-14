#pragma once

#include <drv/IoPin.h>
#include <stdint.h>
#include "usb/driver/nl_usb_midi.h"
#include "usb/driver/nl_usb_core.h"
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

  template <enum USB_BufferType tBufferType>
  class UsbMidiWriter
  {
   private:
    static constexpr unsigned BUFFER_COUNT = unsigned(tBufferType) / sizeof(uint32_t);
    uint32_t *                m_buffer;
    unsigned                  m_bufIndex;
    unsigned                  m_sendBufferIndex;
    IOpins::IOpin &           m_LED_usbStalling;

    inline unsigned modBufferSize(unsigned const x)
    {
      return x & (BUFFER_COUNT - 1);
    };

    inline void advanceIndex(void)
    {
      m_bufIndex = modBufferSize(m_bufIndex + 1u);
    }

    inline unsigned usedBuffer(void)
    {
      return modBufferSize(BUFFER_COUNT + m_bufIndex - m_sendBufferIndex);
    };

   public:
    UsbMidiWriter(uint32_t *const buffer, IOpins::IOpin &LED_usbStalling)
        : m_buffer(buffer)
        , m_LED_usbStalling(LED_usbStalling)
    {
      USB_Core_SetCircularBuffer(0, tBufferType);
    };

    inline int claimBuffer(unsigned const requestedWordCount)
    {
      unsigned freeCount = BUFFER_COUNT - usedBuffer();
      return freeCount > requestedWordCount;
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

    inline void startTransaction(void)
    {
      if (m_sendBufferIndex == m_bufIndex)
        return;
      uint8_t *sendBuffer    = (uint8_t *) (&m_buffer[m_sendBufferIndex]);
      unsigned sendBufferLen = usedBuffer() * sizeof m_buffer[0];

      if (USB_MIDI_Send(0, sendBuffer, sendBufferLen) == -1)
      {  // failed
        m_LED_usbStalling.timedOn(1);
        return;
      }
      m_sendBufferIndex = m_bufIndex;
    };
  };

}  // namespace
