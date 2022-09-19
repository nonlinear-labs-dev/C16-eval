#pragma once

#include <stdint.h>

namespace IOpins
{
  typedef uint32_t volatile* const tIOpinMemoryMapped;

  class IOpin
  {
   private:
    tIOpinMemoryMapped m_ioPinAddress;
    unsigned           m_step = 0;
    unsigned           m_cntr = 0;

   public:
    constexpr IOpin(tIOpinMemoryMapped ioPinAddress)
        : m_ioPinAddress(ioPinAddress) {};

    inline void set(uint32_t const flag) const
    {
      *m_ioPinAddress = flag;
    }

    inline void toggle(void) const
    {
      asm volatile("mov r7, r7");
      (*m_ioPinAddress) = ~(*m_ioPinAddress);
    }

    inline uint32_t get(void) const
    {
      return *m_ioPinAddress;
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
