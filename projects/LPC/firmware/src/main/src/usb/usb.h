#pragma once

#include <stdint.h>
#include "usb/driver/nl_usb_midi.h"
#include "usb/driver/nl_usb_core.h"
#include "drv/allIoPins.h"
#include "tasks/mtask.h"

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

namespace Usb
{

  class UsbMidiWriter
  {
   private:
    uint32_t *const m_buffer;
    unsigned const  m_bufferSize;
    unsigned const  m_bufferCount;
    unsigned        m_bufIndex;
    unsigned        m_sendBufferIndex;

    inline unsigned modBufferSize(unsigned const x)
    {
      return x & (m_bufferCount - 1);
    };

    inline void advanceIndex(void)
    {
      m_bufIndex = modBufferSize(m_bufIndex + 1u);
    }

    inline unsigned usedBuffer(void)
    {
      return modBufferSize(m_bufferCount + m_bufIndex - m_sendBufferIndex);
    }

   public:
    UsbMidiWriter(uint32_t *const buffer, unsigned const bufferSize)
        : m_buffer(buffer)
        , m_bufferSize(bufferSize)
        , m_bufferCount(bufferSize / sizeof m_buffer[0])
    {
      USB_Core_SetCircularBuffer(0, (enum USB_BufferType) m_bufferSize);
    };

    inline int claimBuffer(unsigned const requestedWordCount)
    {
      unsigned freeCount = m_bufferCount - usedBuffer();
      return freeCount > requestedWordCount;
    };

    inline void write(uint8_t const cableNumber, uint8_t const byte1, uint8_t const byte2, uint8_t const byte3)
    {
      uint32_t packet      = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | ((cableNumber << 4) | 0x04);
      m_buffer[m_bufIndex] = packet;
      advanceIndex();
    }

    inline void writeLast(uint8_t const cableNumber, uint8_t const byte1, uint8_t const byte2, uint8_t const byte3)
    {
      uint32_t packet      = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | ((cableNumber << 4) | 0x07);
      m_buffer[m_bufIndex] = packet;
      advanceIndex();
    }

    inline void writeLast(uint8_t const cableNumber, uint8_t const byte1, uint8_t const byte2)
    {
      uint32_t packet      = (byte2 << 16) | (byte1 << 8) | ((cableNumber << 4) | 0x06);
      m_buffer[m_bufIndex] = packet;
      advanceIndex();
    }

    inline void writeLast(uint8_t const cableNumber, uint8_t const byte1)
    {
      uint32_t packet      = (byte1 << 8) | ((cableNumber << 4) | 0x05);
      m_buffer[m_bufIndex] = packet;
      advanceIndex();
    }

    inline void startTransaction(void)
    {
      if (m_sendBufferIndex == m_bufIndex)
        return;
      uint8_t *sendBuffer    = (uint8_t *) (&m_buffer[m_sendBufferIndex]);
      unsigned sendBufferLen = usedBuffer() * sizeof m_buffer[0];
      if (USB_MIDI_Send(0, sendBuffer, sendBufferLen) == -1)
      {  // failed
        LED_USBstalling.timedOn(1);
        return;
      }
      m_sendBufferIndex = m_bufIndex;
    }
  };

}  // namespace

extern Usb::UsbMidiWriter sensorAndKeyEventWriter;
