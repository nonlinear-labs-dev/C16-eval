#pragma once

namespace Task
{

  class UsbProcess : public Task::Task
  {
    using Task::Task;

   private:
    Usb::UsbMidiSysexWriter& m_sensorAndKeyEventWriter;
    Usb::UsbBridgeWriter&    m_bridgeToHost;
    Usb::UsbBridgeWriter&    m_hostToBridge;

   public:
    constexpr UsbProcess(Usb::UsbMidiSysexWriter& usbMidiWriter,
                         Usb::UsbBridgeWriter&    bridgeToHost,
                         Usb::UsbBridgeWriter&    hostToBridge)
        : m_sensorAndKeyEventWriter(usbMidiWriter)
        , m_bridgeToHost(bridgeToHost)
        , m_hostToBridge(hostToBridge)
    {
      Usb::initBridges(bridgeToHost, hostToBridge);
    };

    // no dispatcher and body needed
    inline void dispatch(void) {};
    inline void body(void) {};

    // run unconditionally
    inline void run(void) const
    {
      m_sensorAndKeyEventWriter.processPendingTransactions();
      if (m_sensorAndKeyEventWriter.isIdle())
        m_bridgeToHost.processPendingTransactions();
      m_hostToBridge.processPendingTransactions();
    };
  };

}
