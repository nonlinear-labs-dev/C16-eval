#pragma once

#include <stdint.h>
#include "usb/usbWriter_StateMachine.h"

namespace UsbWriter
{

  class BridgeWriter : public WriterStateMachine
  {
   public:
    constexpr BridgeWriter(HardwareAccess &hwAccess)
        : WriterStateMachine(hwAccess)
        , m_incomingPort(1 - hwAccess.getPort()) {};

   private:
    // ---- implemented virtuals
    bool waitingForUserDataReady(void) override
    {
      return m_pData == nullptr;
    };

    void userTransactionPreStart(void) override
    {
      if (waitingForUserDataReady())
        return;
      setupTransmitData((void *) m_pData, m_dataSize, USBTimeouts::UseTimeout);
    };

    void onGoingOnline(void) override
    {
      USB_MIDI_SuspendReceive(m_hwAccess.getPort(), 0);  // re-enable receiver
      USB_MIDI_ClearReceive(m_hwAccess.getPort());
      USB_MIDI_primeReceive(m_hwAccess.getPort());
    };

    void finishUserTransaction(void) override
    {
      USB_MIDI_SuspendReceive(m_incomingPort, 0);  // re-enable receiver
      USB_MIDI_ClearReceive(m_incomingPort);
      USB_MIDI_primeReceive(m_incomingPort);
      m_pData = nullptr;
    };

    void abortUserTransaction(void) override
    {
      finishUserTransaction();
    };

    // ---- local stuff ----
    uint8_t        m_incomingPort;
    uint8_t const *m_pData { nullptr };
    uint16_t       m_dataSize { 0 };

   public:
    void onReceive(uint8_t const *const pData, uint16_t const length)  // veneer-called from USB receive interrupt
    {
      if (length == 0)
        return;  // empty packet will not cause any action and is simply ignored

      if (m_pData != nullptr)  // we should never receive a packet when a transmit is in process
        return;

      m_pData    = pData;
      m_dataSize = length;
      USB_MIDI_SuspendReceive(m_incomingPort, 1);  // block receiver until transmit finished/failed
    };

  };  // class BridgeWriter

}  // namespace UsbWriter
