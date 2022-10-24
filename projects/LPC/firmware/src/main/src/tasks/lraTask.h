#pragma once

#include "io/pin_setup.h"
#include "drv/IoPin.h"
#include "drv/LRA.h"
#include "tasks/mtask.h"

namespace Task
{

  class LRAHandler : public Task::Task
  {
   private:
    IOpins::IOpin&   m_lraActivityLED;
    LraHardware::LRA m_lra[8] { &pinLRA_0, &pinLRA_1, &pinLRA_2, &pinLRA_3, &pinLRA_4, &pinLRA_5, &pinLRA_6, &pinLRA_7 };
#if 0
      // start of test code
    unsigned       buttons { 0 };
    unsigned       lraNum { 0 };
    // end of test code
#endif

    void stopAll(void)
    {
      pinLRA_nENABLE = !0;
      m_lra[0].reset();
      m_lra[1].reset();
      m_lra[2].reset();
      m_lra[3].reset();
      m_lra[4].reset();
      m_lra[5].reset();
      m_lra[6].reset();
      m_lra[7].reset();
    };

   public:
    constexpr LRAHandler(uint32_t const delay, uint32_t const period, IOpins::IOpin& lraActivityLED)
        : Task(delay, period)
        , m_lraActivityLED(lraActivityLED) {};

    inline void startPattern(uint8_t const lraNumAndPatternId)
    {
      unsigned lraNum    = lraNumAndPatternId >> 5;
      unsigned patternId = lraNumAndPatternId & 0b00011111;
      uint64_t pattern   = LraHardware::off;

      for (unsigned i = 0; i < (sizeof LraHardware::patternLookupTable / sizeof LraHardware::patternLookupTable[0]); i++)
      {
        if (patternId == unsigned(LraHardware::patternLookupTable[i].patternId))
        {
          pattern = LraHardware::patternLookupTable[i].pattern;
          break;
        }
      }

      m_lra[lraNum].startPattern(pattern);
    };

    inline void body(void)
    {

#if 0
      // start of test code
      unsigned newButtons = IOpins::readButtons();
      if (buttons == 0 && newButtons == 0b0001)
        m_lra[lraNum].startPattern(LRA::blip_l4_s4);
      else if (buttons == 0 && newButtons == 0b0010)
        m_lra[lraNum].startPattern(LRA::cont_s1);
      else if (buttons == 0 && newButtons == 0b0100)
        stopAll();
      else if (buttons == 0 && newButtons == 0b1000)
      {
        stopAll();
        lraNum = (lraNum + 1) % 6;
      }
      else if (buttons == 0b1010 && newButtons == 0b1010)
      {
        while (1)  // catastrophic errors test
          *((volatile uint32_t*) buttons++) = 0;
      }

      buttons = newButtons;
      // end of test code
#endif

      int activity   = (m_lra[0].process() + m_lra[1].process() + m_lra[2].process() + m_lra[3].process()
                      + m_lra[4].process() + m_lra[5].process() + m_lra[6].process() + m_lra[7].process());
      pinLRA_nENABLE = !activity;
      if (activity)
        m_lraActivityLED.timedOn(1);
    }
  };

}
