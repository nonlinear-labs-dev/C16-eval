#pragma once

extern Usb::UsbMidiWriter sensorAndKeyEventWriter;

namespace Task
{
  class UsbWriter : public Task::Task
  {
    using Task::Task;

   public:
    // no dispatcher and body needed
    inline void dispatch(void)
    {
    }

    inline void body(void)
    {
    }

    // run unconditionally
    inline void run(void)
    {
      sensorAndKeyEventWriter.startTransaction();
    };
  };

}
