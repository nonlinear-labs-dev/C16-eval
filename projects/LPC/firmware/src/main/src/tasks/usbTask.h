#pragma once

namespace Task
{
  class UsbProcess : public Task::Task
  {
    using Task::Task;

   private:
    Usb::UsbMidiSysexWriter& m_sensorAndKeyEventWriter;

   public:
    constexpr UsbProcess(Usb::UsbMidiSysexWriter& usbMidiWriter)
        : m_sensorAndKeyEventWriter(usbMidiWriter) {};

    // no dispatcher and body needed
    inline void dispatch(void) {};
    inline void body(void) {};

    // run unconditionally
    inline void run(void) const
    {
      m_sensorAndKeyEventWriter.processTransaction();
    };
  };

}
