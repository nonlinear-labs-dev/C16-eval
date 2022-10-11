#pragma once

#include "tasks/mtask.h"
#include "drv/IoPin.h"
#include "io/pins.h"

namespace Task
{

  class AllIoPins : public Task::Task
  {
    using Task::Task;

   public:
    IOpins::IOpin m_LED_m4HeartBeat { pinLED_M4HB };
    IOpins::IOpin m_LED_warning { pinLED_WARNING };
    IOpins::IOpin m_LED_error { pinLED_ERROR };
    IOpins::IOpin m_LED_keybedEvent { pinLED_E };
    IOpins::IOpin m_LED_uartActivity { pinLED_F };
    IOpins::IOpin m_LED_uartError { pinLED_J };
    IOpins::IOpin m_LED_usbDelayedPacket { pinLED_H };
    IOpins::IOpin m_LED_adcOverrun { pinLED_I };

    inline void body(void)
    {
      // m_LED_m4HeartBeat.process();  // not used
      m_LED_warning.process();
      m_LED_error.process();
      m_LED_keybedEvent.process();
      m_LED_uartActivity.process();
      m_LED_uartError.process();
      m_LED_usbDelayedPacket.process();
      m_LED_adcOverrun.process();
    };
  };
}
