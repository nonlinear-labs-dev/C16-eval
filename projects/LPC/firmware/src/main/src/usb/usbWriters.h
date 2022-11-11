#pragma once

#include <stdint.h>
#include "drv/IoPin.h"
#include "usb/driver/nl_usb_midi.h"
#include "usb/driver/nl_usb_core_circular_buffers.h"
#include "usb/driver/nl_usb_descmidi.h"

namespace Usb
{
  enum class USBPorts
  {
    USB0 = 0,
    USB1 = 1,
  };

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

  class UsbMidiSysexWriter
  {
   private:
    static constexpr unsigned   BUFFER_ELEM_COUNT { sizeof(USB_BUFFER_FOR_SENSOR_DATA) / sizeof(USB_BUFFER_FOR_SENSOR_DATA[0]) };
    uint32_t *                  m_buffer { &USB_BUFFER_FOR_SENSOR_DATA[0] };
    unsigned                    m_bufIndex { 0 };
    unsigned                    m_sendBufferIndex { 0 };
    unsigned                    m_currentTransactionElemCount { 0 };
    uint8_t                     m_outgoingPort;
    StateMonitor::StateMonitor &m_stateMonitor;

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
    constexpr UsbMidiSysexWriter(enum Usb::USBPorts const outgoingPort, StateMonitor::StateMonitor &stateMonitor)
        : m_outgoingPort(outgoingPort == Usb::USBPorts::USB0 ? 0 : 1)
        , m_stateMonitor(stateMonitor) {};

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

   private:
    enum class States
    {
      IDLE,
      WAIT_FOR_XMIT_READY,
      WAIT_FOR_XMIT_DONE,
    };
    enum States m_state
    {
      States::IDLE
    };

   public:
    inline int isIdle(void)
    {
      return m_state == States::IDLE;
    };

    inline void processPendingTransactions(void)
    {
      uint8_t *sendBuffer;
      unsigned sendBufferLen;
      while (1)
      {
        switch (m_state)
        {
          case States::IDLE:
            if (m_sendBufferIndex == m_bufIndex)
              return;
            m_state = States::WAIT_FOR_XMIT_READY;
            // intentional fall-through

          case States::WAIT_FOR_XMIT_READY:
            sendBuffer    = (uint8_t *) (&m_buffer[m_sendBufferIndex]);
            sendBufferLen = usedBuffer() * sizeof m_buffer[0];
            if (USB_MIDI_Send(m_outgoingPort, sendBuffer, sendBufferLen) == -1)
            {  // failed
              m_stateMonitor.event(StateMonitor::WARNING_USB_DELAYED_PACKET);
              return;
            }
            m_state                       = States::WAIT_FOR_XMIT_DONE;
            m_sendBufferIndex             = m_bufIndex;
            m_currentTransactionElemCount = sendBufferLen / sizeof m_buffer[0];
            // intentional fall-through

          case States::WAIT_FOR_XMIT_DONE:
            if (USB_MIDI_BytesToSend(m_outgoingPort) > 0)
              return;
            m_state                       = States::IDLE;
            m_currentTransactionElemCount = 0;
            // check for pending transfer immediately to avoid an IDLE glitch
        }
      }
    };

  };  // class UsbMidiSysexWriter

  // ---------------------------------- // ---------------------------------- // ----------------------------------

  class UsbBridgeWriter
  {
   private:
    uint8_t                     m_outgoingPort;
    uint8_t                     m_incomingPort;
    StateMonitor::StateMonitor &m_stateMonitor;

   public:
    constexpr UsbBridgeWriter(enum Usb::USBPorts const outgoingPort, StateMonitor::StateMonitor &stateMonitor)
        : m_outgoingPort(outgoingPort == Usb::USBPorts::USB0 ? 0 : 1)
        , m_incomingPort(1 - m_outgoingPort)
        , m_stateMonitor(stateMonitor) {};

   private:
    enum class States
    {
      IDLE,
      DATA_RECEIVED,
      WAIT_FOR_XMIT_READY,
      WAIT_FOR_XMIT_DONE,
    };
    enum States m_state
    {
      States::IDLE
    };
    uint8_t *m_pData { nullptr };
    uint16_t m_dataSize { 0 };

   public:
    inline void processPendingTransactions(void)
    {

      if (m_state == States::IDLE)
        return;

      switch (m_state)
      {
        default:
          return;

        case States::DATA_RECEIVED:
          m_state = States::WAIT_FOR_XMIT_READY;
          // fall-through is on purpose

        case States::WAIT_FOR_XMIT_READY:
          if (USB_MIDI_Send(m_outgoingPort, m_pData, m_dataSize) == -1)
          {  // failed
            m_stateMonitor.event(StateMonitor::WARNING_USB_DELAYED_PACKET);
            return;  // could not start transfer now, try later
          }
          m_state = States::WAIT_FOR_XMIT_DONE;
          return;

        case States::WAIT_FOR_XMIT_DONE:
          if (USB_MIDI_BytesToSend(m_outgoingPort) > 0)
            return;  // still sending...
          m_state = States::IDLE;
          USB_MIDI_SuspendReceive(m_incomingPort, 0);  // re-enable receiver
          USB_MIDI_primeReceive(m_incomingPort);
          return;
      }
    };

    inline void onReceive(uint8_t *pData, uint16_t length)
    {
      if (length == 0)
        return;  // empty packet will not cause any action and is simply ignored

      if (m_state != States::IDLE)  // we should never receive a packet when not IDLE
        return;

      m_pData    = pData;
      m_dataSize = length;
      m_state    = States::DATA_RECEIVED;
      USB_MIDI_SuspendReceive(m_incomingPort, 1);  // block receiver until transmit finished/failed
    };

  };  // class UsbBridgeWriter

  // ---------------------------------- // ---------------------------------- // ----------------------------------

  static UsbBridgeWriter *pBridgeToHost;
  static UsbBridgeWriter *pHostToBridge;

  // receive from Host Callback
  static void Receive_IRQ_Callback_0(uint8_t const port, uint8_t *buff, uint32_t len)
  {
    pHostToBridge->onReceive(buff, (uint16_t) len);
  }

  // receive from Bridge Callback
  static void Receive_IRQ_Callback_1(uint8_t const port, uint8_t *buff, uint32_t len)
  {
    pBridgeToHost->onReceive(buff, (uint16_t) len);
  }

  inline void initBridges(void)
  {
    USB_MIDI_Config(0, Receive_IRQ_Callback_0);
    USB_MIDI_Config(1, Receive_IRQ_Callback_1);
    USB_MIDI_SetupDescriptors();
    USB_MIDI_Init(0);
    USB_MIDI_Init(1);
  }

  inline void initBridges(UsbBridgeWriter &bridgeToHost, UsbBridgeWriter &hostToBridge)
  {
    pBridgeToHost = &bridgeToHost;
    pHostToBridge = &hostToBridge;
    initBridges();
  }

}  // namespace
