#pragma once

#include <stdint.h>
#include "sys/ticker.h"
#include "usb/driver/nl_usb_midi.h"
#include "tasks/statemonitor.h"

namespace UsbWriter
{
  enum class USBPorts
  {
    USB0 = 0,
    USB1 = 1,
  };

  typedef uint64_t       tTime;
  static constexpr tTime TIMEOUT_TIMEOUT_INITIAL    = msToTicks(400);
  static constexpr tTime TIMEOUT_TIMEOUT_SUBSEQUENT = msToTicks(10);

  class HardwareAccess
  {

   public:
    constexpr HardwareAccess(enum USBPorts const outgoingPort, StateMonitor::StateMonitor &stateMonitor)
        : m_stateMonitor(stateMonitor)
        , m_outgoingPort(outgoingPort == USBPorts::USB0 ? 0 : 1)
    {
      USB_MIDI_Init(m_outgoingPort);
    };

    constexpr uint8_t getPort(void) const
    {
      return m_outgoingPort;
    };

    bool isOnline(void) const
    {
      return m_enabled && USB_MIDI_IsConfigured(m_outgoingPort);
    };

    bool isBusy(void) const
    {
      return m_busy;
    };

    void enable(bool const enable)
    {
      m_enabled = enable;
    };

    void config(uint8_t const configByte)
    {
      enable((configByte & (1 << 0)) != 0);  // bit 0 : enable
      if ((configByte & (1 << 1)) != 0)      // bit 1 : clear flags (!! note: only ONE set of flags for both USBs !!)
        m_stateMonitor.clearUsbFlags(m_outgoingPort);
    };

    void setupTransactionData(void *const pData, uint16_t const dataSize, bool const useTimeout)
    {
      m_useTimeout = useTimeout;
      m_pData      = (uint8_t const *const) pData;
      m_dataSize   = dataSize;
    };

    bool prepareTransaction(void)
    {
      if (m_busy)
        return false;
      if (m_useTimeout)
      {
        m_packetTimeout = !m_dropped ? TIMEOUT_TIMEOUT_INITIAL : TIMEOUT_TIMEOUT_SUBSEQUENT;
        m_dropped       = false;
        m_packetTime    = ticker;
      }
      return true;
    };

    bool startTransaction(void)
    {
      m_busy = true;
      if (isOnline())
        return USB_MIDI_Send(m_outgoingPort, m_pData, m_dataSize) != -1;
      return true;
    };

    bool transactionFinished(void)
    {
      if (isOnline())
        return !(m_busy = USB_MIDI_BytesToSend(m_outgoingPort) > 0);
      m_busy = false;
      return true;
    };

    bool timedOut(void) const
    {
      if (!m_useTimeout)
        return false;
      return (ticker - m_packetTime) > m_packetTimeout;
    };

    void killTransaction(void)
    {
      USB_MIDI_KillTransmit(m_outgoingPort);
      m_dropped = true;
      m_busy    = false;
    };

    // ---- data members ----
    StateMonitor::StateMonitor &m_stateMonitor;

   private:
    bool  m_enabled { true };
    bool  m_busy { false };
    bool  m_useTimeout { false };
    tTime m_packetTime { 0 };
    tTime m_packetTimeout { TIMEOUT_TIMEOUT_INITIAL };
    bool  m_dropped { false };

    uint8_t        m_outgoingPort;
    uint8_t const *m_pData { nullptr };
    uint16_t       m_dataSize { 0 };

  };  // class HardwareAccess

}  // namespace UsbWriter
