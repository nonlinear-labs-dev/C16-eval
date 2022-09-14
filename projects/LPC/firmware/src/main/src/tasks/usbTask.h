#pragma once

namespace Task
{
  class UsbProcess : public Task::Task
  {
    using Task::Task;

   private:
    Usb::UsbMidiWriter_16kBuffer& m_sensorAndKeyEventWriter;

   public:
    UsbProcess(Usb::UsbMidiWriter_16kBuffer& usbMidiWriter)
        : m_sensorAndKeyEventWriter(usbMidiWriter) {};

    // no dispatcher and body needed
    inline void dispatch(void) {};

    inline void body(void) {};

    // run unconditionally
    inline void run(void)
    {
      m_sensorAndKeyEventWriter.startTransaction();
    };
  };

}
