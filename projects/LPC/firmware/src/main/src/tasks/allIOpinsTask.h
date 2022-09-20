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
    IOpins::IOpin m_LED_m4HeartBeat { LED_M4HB };
    IOpins::IOpin m_LED_warning { LED_WARNING };
    IOpins::IOpin m_LED_error { LED_ERROR };
    IOpins::IOpin m_LED_keybedEvent { LED_E };
    IOpins::IOpin m_LED_usbStalling { LED_H };
    IOpins::IOpin m_LED_adcOverrun { LED_I };

    inline void body(void)
    {
      // m_LED_m4HeartBeat.process();  // not used
      m_LED_warning.process();
      m_LED_error.process();
      m_LED_keybedEvent.process();
      m_LED_usbStalling.process();
      m_LED_adcOverrun.process();
    };
  };
}
