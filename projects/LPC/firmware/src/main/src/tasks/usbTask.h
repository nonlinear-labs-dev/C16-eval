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
    constexpr UsbProcess(tUsbMidiWriter& usbMidiWriter)
        : m_sensorAndKeyEventWriter(usbMidiWriter) {};

    // no dispatcher and body needed
    inline void dispatch(void) const {};
    inline void body(void) const {};

    // run unconditionally
    inline void run(void) const
    {
      m_sensorAndKeyEventWriter.processTransaction();
    };
  };

}
