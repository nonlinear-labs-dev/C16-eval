#pragma once

#include <stdint.h>
#include "io/pins.h"

namespace IOpins
{

  inline uint32_t readButtons(void)
  {
    return 0b11111 ^ (pinENC_BUTTON << 4 | pinBUTTON_D << 3 | pinBUTTON_C << 2 | pinBUTTON_B << 1 | pinBUTTON_A << 0);
  }

  class IOpin
  {
   private:
    typedef uint32_t volatile t_ioPin;
    t_ioPin &m_ioPin;
    unsigned m_step { 0 };
    unsigned m_cntr { 0 };

   public:
    constexpr IOpin(t_ioPin &ioPin)
        : m_ioPin(ioPin) {};

    inline void set(uint32_t const flag) const
    {
      m_ioPin = flag;
    }

    inline void toggle(void) const
    {
      m_ioPin = ~m_ioPin;
    }

    inline uint32_t get(void) const
    {
      return m_ioPin;
    }

    inline void timedOn(int ticks)
    {
      if (ticks == 0)
        return;

      if (ticks < 0)  // continuous mode
      {
        m_cntr = -ticks;
        m_step = 1;
        set(1);
      }
      else  // flicker when re-triggered
      {
        m_cntr = ticks;
        if (m_step == 0)  // not already running?
        {
          m_step = 1;
          set(1);
        }
        else if (m_step == 1)  // in normal mode ?
          m_step = 2;
      }
    }

    inline void process(void)
    {
      switch (m_step)
      {
        case 0:
          return;

        case 1:
          if (m_cntr)
          {
            set(1);
            if (!--m_cntr)
            {
              set(0);
              m_step = 0;
            }
          }
          return;

        case 2:
          set(0);
          m_step++;
          return;

        case 3:
          set(m_step = (m_cntr != 0));
          return;
      }
    }
  };

}  // namespace
