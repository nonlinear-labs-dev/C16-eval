#pragma once

#include <stdint.h>
#include "io/pins.h"
#include "ipc/ipc.h"

namespace Task
{

  class Task
  {
   private:
    unsigned m_period;
    unsigned m_cntr;
    int      m_start;
    unsigned m_taskMaxTime;

   public:
    // High priority task with no start delay and no time-slicing
    // Usually, these should overload dispatch() and body() with a dummy and run()
    // with the actual body that does the work. This avoids superfluous code
    Task(void)
        : m_period(0)
        , m_cntr(0) {};

    Task(uint32_t const delay, uint32_t const period)
        : m_period(period)   // period is the repeat time in ticks, must be > 0
        , m_cntr(delay) {};  // delay is the amount of ticks until first start

    inline virtual void body(void) {};

    // mark task for execution when due (non-cumulative)
    inline virtual void dispatch(void)
    {
      if (m_cntr)
      {
        if (--m_cntr)
          return;
      }

      if (m_start && m_period)  // overrun
      {
        PINS_CriticalPinsInit();
        __asm volatile("cpsid i");
        while (1)
          LED_ERROR = 1;
      }

      m_cntr  = m_period;
      m_start = 1;
    }

    inline virtual void run(void)
    {
      if (m_start)
      {
        m_start = 0;
        body();
        unsigned taskTime = 1 + s.timesliceTicker5us;
        if (taskTime > m_taskMaxTime)
          m_taskMaxTime = taskTime;
      }
    }
  };

}
