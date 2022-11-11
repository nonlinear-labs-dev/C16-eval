#pragma once

#include "usb/usbWriter.h"
#include "tasks/statemonitor.h"

namespace UsbWriter
{

  class BridgeDataUsbWriter : UsbWriter
  {
   public:
    constexpr BridgeDataUsbWriter(enum USBPorts const outgoingPort, StateMonitor::StateMonitor &stateMonitor)
        : m_outgoingPort(outgoingPort == USBPorts::USB0 ? 0 : 1)
        , m_incomingPort(1 - m_outgoingPort)
        , m_stateMonitor(stateMonitor) {};

    inline void process(void)
    {
      uint8_t online = (USB_MIDI_IsConfigured(m_outgoingPort) != 0);
      if (online != m_online)
      {
        m_online = online;
        if (online)
          enableReceiver();
        else
          packetTransferReset();
      }

      if (online)
        stateMachine();
    }

    inline void onReceive(uint8_t const *const pData, uint16_t const length)
    {
      if (length == 0)
        return;  // empty packet will not cause any action and is simply ignored

      if (USB_MIDI_IsConfigured(m_incomingPort) == 0)
        return;  // just in case incoming port went offline and we still got an interrupt

      if (!isIdle())  // we should never receive a packet when not IDLE
        return;

      if (!m_online)
        return;  // outgoing port is offline, packet will be dismissed

      m_pData    = pData;
      m_dataSize = length;
      m_haveData = true;
      disableReceiver();
    };

   private:
    // ---- implementing virtuals ----
    bool waitingForDataReady(void)
    {
      if (m_haveData)
      {
        m_haveData = false;
        return false;
      }
      return true;
    };

    bool waitingForTransmitStarted(void) const
    {
      if (USB_MIDI_Send(m_outgoingPort, m_pData, m_dataSize) == -1)
      {  // failed
        m_stateMonitor.event(StateMonitor::WARNING_USB_DELAYED_PACKET);
        return true;
      }
      return false;
    };

    bool waitingForTransmitDone(void) const
    {
      if (USB_MIDI_BytesToSend(m_outgoingPort) > 0)
        return true;  // still sending...
      enableReceiver();
      return false;
    };

    bool processOnlyOneTransmitAtATime(void) const
    {
      return true;
    };

    void killTransfer(void)
    {
      USB_MIDI_KillTransmit(m_outgoingPort);
      enableReceiver();
    };

    // ---- data members ----
    uint8_t                     m_outgoingPort;
    uint8_t                     m_incomingPort;
    StateMonitor::StateMonitor &m_stateMonitor;
    uint8_t const *             m_pData { nullptr };
    uint16_t                    m_dataSize { 0 };
    bool                        m_haveData { false };
    uint8_t                     m_online { 0xAA };

    // ---- helpers ----
    inline void packetTransferReset(void)
    {
      resetTransfer();
      enableReceiver();
      m_haveData = false;
    };

    inline void enableReceiver(void) const
    {
      USB_MIDI_SuspendReceive(m_incomingPort, 0);  // enable receiver...
      USB_MIDI_primeReceive(m_incomingPort);       // ...and arm it
    }

    inline void disableReceiver(void) const
    {
      USB_MIDI_SuspendReceive(m_incomingPort, 1);  // block receiver until transmit finished/failed
    }
  };

}  // namespace UsbWriter
