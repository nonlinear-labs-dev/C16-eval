#pragma once

#include <stdint.h>
#include "usb/driver/nl_usb_midi.h"
#include "usb/driver/nl_usb_core.h"

namespace Usb
{

  static inline unsigned mod1024(unsigned const x)
  {
    return (x & 1023u);
  }

  class UsbMidiSysexWriter
  {
   private:
    uint32_t *const m_buffer;
    unsigned        m_bufIndex;
    unsigned        m_sendBufferIndex;

   public:
    UsbMidiSysexWriter(uint32_t *const buffer)
        : m_buffer(buffer)
    {
      USB_Core_SetWrapping4kBuffer(0, 1);
    };

    inline void advanceIndex(void)
    {
      m_bufIndex = mod1024(m_bufIndex + 1u);
    }

    inline void start(void)
    {
      m_sendBufferIndex = m_bufIndex;
    }

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

    int send(void)
    {
      int      result;
      uint8_t *sendBuffer    = (uint8_t *) (&m_buffer[m_sendBufferIndex]);
      unsigned sendBufferLen = mod1024(1024u + m_bufIndex - m_sendBufferIndex) * sizeof m_buffer[0];
      if ((result = USB_MIDI_Send(0, sendBuffer, sendBufferLen)) == -1)
      {  // failed, re-use previous buffer;
        m_bufIndex = m_sendBufferIndex;
      }
      return result;
    }
  };

}  // namespace

extern Usb::UsbMidiSysexWriter sensorAndKeyEventWriter;
