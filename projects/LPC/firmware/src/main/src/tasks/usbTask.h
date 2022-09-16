#pragma once

namespace Task
{
  template <typename tUsbMidiWriter>
  class UsbProcess : public Task::Task
  {
    using Task::Task;

   private:
    tUsbMidiWriter& m_sensorAndKeyEventWriter;

   public:
    UsbProcess(tUsbMidiWriter& usbMidiWriter)
        : m_sensorAndKeyEventWriter(usbMidiWriter) {};

    // no dispatcher and body needed
    inline void dispatch(void) {};
    inline void body(void) {};

    // run unconditionally
    inline void run(void)
    {
      m_sensorAndKeyEventWriter.processTransaction();
    };
  };

}
