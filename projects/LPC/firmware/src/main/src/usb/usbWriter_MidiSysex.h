#pragma once

#include <stdint.h>
#include "usb/driver/nl_usb_core_circular_buffers.h"
#include "usb/usbWriter_StateMachine.h"

namespace UsbWriter
{

  class MidiSysexWriter : public WriterStateMachine
  {
   public:
    constexpr MidiSysexWriter(HardwareAccess &hwAccess)
        : WriterStateMachine(hwAccess) {};

   private:
    // ---- implemented virtuals
    bool processOnlyOneTransmitAtATime(void) override
    {
      return false;
    };

    bool waitingForUserDataReady(void) override
    {
      return m_sendBufferIndex == m_bufIndex;
    };

    void userTransactionPreStart(void) override
    {
      if (waitingForUserDataReady())
        return;
      m_currentTransactionElemCount = usedBuffer();
      setupTransmitData((void *) (&m_buffer[m_sendBufferIndex]), uint16_t(m_currentTransactionElemCount * sizeof m_buffer[0]), USBTimeouts::UseTimeout);
    };

    void userTransactionPostStart(void) override
    {
      m_sendBufferIndex = m_bufIndex;
    };

    void finishUserTransaction(void) override
    {
      m_currentTransactionElemCount = 0;
    };

    void abortUserTransaction(void) override
    {
      finishUserTransaction();
    };

    // --- interface functions ----
   public:
    bool claimBufferElements(unsigned const requestedElemCount) const
    {
      unsigned freeCount = BUFFER_ELEM_COUNT - usedBuffer() - m_currentTransactionElemCount;
      return freeCount > requestedElemCount;
    };

    void write(uint8_t const cableNumber, uint8_t const byte1, uint8_t const byte2, uint8_t const byte3)
    {
      uint32_t packet      = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | ((cableNumber << 4) | 0x04);
      m_buffer[m_bufIndex] = packet;
      advanceIndex();
    };

    void writeLast(uint8_t const cableNumber, uint8_t const byte1, uint8_t const byte2, uint8_t const byte3)
    {
      uint32_t packet      = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | ((cableNumber << 4) | 0x07);
      m_buffer[m_bufIndex] = packet;
      advanceIndex();
    };

    void writeLast(uint8_t const cableNumber, uint8_t const byte1, uint8_t const byte2)
    {
      uint32_t packet      = (byte2 << 16) | (byte1 << 8) | ((cableNumber << 4) | 0x06);
      m_buffer[m_bufIndex] = packet;
      advanceIndex();
    };

    void writeLast(uint8_t const cableNumber, uint8_t const byte1)
    {
      uint32_t packet      = (byte1 << 8) | ((cableNumber << 4) | 0x05);
      m_buffer[m_bufIndex] = packet;
      advanceIndex();
    };

    // ---- local stuff ----
   private:
    static constexpr unsigned BUFFER_ELEM_COUNT { sizeof(USB_BUFFER_FOR_SENSOR_DATA) / sizeof(USB_BUFFER_FOR_SENSOR_DATA[0]) };
    uint32_t *                m_buffer { &USB_BUFFER_FOR_SENSOR_DATA[0] };
    unsigned                  m_bufIndex { 0 };
    unsigned                  m_sendBufferIndex { 0 };
    unsigned                  m_currentTransactionElemCount { 0 };

    unsigned usedBuffer(void) const
    {
      return modBufferSize(BUFFER_ELEM_COUNT + m_bufIndex - m_sendBufferIndex);
    };

    // relies on buffer sizes being 2^N for efficiency (modulo operator optimized out)
    unsigned modBufferSize(unsigned const x) const
    {
      return x % BUFFER_ELEM_COUNT;
    };

    void advanceIndex(void)
    {
      m_bufIndex = modBufferSize(m_bufIndex + 1u);
    }

  };  // class MidiSysexWriter

}  // namespace UsbWriter
