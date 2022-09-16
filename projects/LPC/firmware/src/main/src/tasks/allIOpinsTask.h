#pragma once

#include "tasks/mtask.h"
#include "drv/IoPin.h"
#include "io/pins.h"

namespace Task
{

  class AllIoPins : public Task::Task
  {
    using Task::Task;

   private:
    IOpins::IOpin m_LED_m4HeartBeat;
    IOpins::IOpin m_LED_warning;
    IOpins::IOpin m_LED_error;
    IOpins::IOpin m_LED_keybedEvent;
    IOpins::IOpin m_LED_usbStalling;
    IOpins::IOpin m_LED_adcOverrun;

   public:
    AllIoPins(uint32_t const delay, uint32_t const period)
        : Task(delay, period)
        , m_LED_m4HeartBeat(&LED_M4HB)
        , m_LED_warning(&LED_WARNING)
        , m_LED_error(&LED_ERROR)
        , m_LED_keybedEvent(&LED_E)
        , m_LED_usbStalling(&LED_H)
        , m_LED_adcOverrun(&LED_I) {};

    inline void body(void)
    {
      // m_LED_m4HeartBeat.process();  // not used
      m_LED_warning.process();
      m_LED_error.process();
      m_LED_keybedEvent.process();
      m_LED_usbStalling.process();
      m_LED_adcOverrun.process();
    };

    inline IOpins::IOpin& getLED_m4HeartBeat(void)
    {
      return m_LED_m4HeartBeat;
    }

    inline IOpins::IOpin& getLED_warning(void)
    {
      return m_LED_warning;
    }

    inline IOpins::IOpin& getLED_error(void)
    {
      return m_LED_error;
    }

    inline IOpins::IOpin& getLED_keybedEvent(void)
    {
      return m_LED_keybedEvent;
    }

    inline IOpins::IOpin& getLED_usbStalling(void)
    {
      return m_LED_usbStalling;
    }

    inline IOpins::IOpin& getLED_adcOverrun(void)
    {
      return m_LED_adcOverrun;
    }
  };

}
