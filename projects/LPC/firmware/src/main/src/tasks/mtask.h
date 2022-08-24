#pragma once

#include <stdint.h>

namespace Task
{

  class Task
  {
   private:
    unsigned          m_delay;
    unsigned          m_period;
    volatile unsigned m_cntr;
    volatile int      m_start;

   public:
    Task(uint32_t delay, uint32_t period)
        : m_delay(delay)
        , m_period(period)
        , m_cntr(0)
        , m_start(0) {};

    inline virtual void body(void) {};

    inline void dispatch(void)
    {
      if (m_delay)
      {
        m_delay--;
        return;
      }
      if (m_cntr)
        m_cntr--;
      else
      {
        m_cntr  = m_period;
        m_start = 1;
      }
    }

    inline int run(void)
    {
      if (m_start)
      {
        m_start = 0;
        body();
        return 1;
      }
      return 0;
    }
  };

}
