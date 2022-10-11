#pragma once

#include "drv/IoPin.h"
#include "tasks/mtask.h"

namespace Task
{

  namespace LRA
  {
    static constexpr unsigned resonanceFrequency            = 165ul;
    static constexpr unsigned slicesPerPeriod               = 16ul;  // 8 slices per half-cycle to allow for different strengths
    static constexpr unsigned resonancePeriodInMicroseconds = 1000000ul / resonanceFrequency / slicesPerPeriod;

    // clang-format off

    // pattern arrays (are read out MSB first)
    // lowest three bits are the sequence length in periods, 1..7, 0 is special and means infinite repeat
    //                                         7654321|7654321|7654321|7654321|7654321|7654321|7654321|76543000 + len

    // off                              7654321|7654321|7654321|7654321|7654321|7654321|7654321|76543000 + len
    static constexpr uint64_t off = { 0b0000000000000000000000000000000000000000000000000000000000000000 + 1 };

    // single blips                            7654321|7654321|7654321|7654321|7654321|7654321|7654321|76543000 + len
    static constexpr uint64_t blip_l1_s1 = { 0b1000000000000000000000000000000000000000000000000000000000000000 + 1 };
    static constexpr uint64_t blip_l1_s2 = { 0b1100000000000000000000000000000000000000000000000000000000000000 + 1 };
    static constexpr uint64_t blip_l1_s3 = { 0b1110000000000000000000000000000000000000000000000000000000000000 + 1 };
    static constexpr uint64_t blip_l1_s4 = { 0b1111110000000000000000000000000000000000000000000000000000000000 + 1 };

    // double blips                            7654321|7654321|7654321|7654321|7654321|7654321|7654321|76543000 + len
    static constexpr uint64_t blip_l2_s1 = { 0b1000000000000000100000000000000000000000000000000000000000000000 + 2 };
    static constexpr uint64_t blip_l2_s2 = { 0b1100000000000000110000000000000000000000000000000000000000000000 + 2 };
    static constexpr uint64_t blip_l2_s3 = { 0b1110000000000000111000000000000000000000000000000000000000000000 + 2 };
    static constexpr uint64_t blip_l2_s4 = { 0b1111110000000000111111000000000000000000000000000000000000000000 + 2 };

    // quadruple blips                         7654321|7654321|7654321|7654321|7654321|7654321|7654321|76543000 + len
    static constexpr uint64_t blip_l4_s1 = { 0b1000000000000000100000000000000010000000000000001000000000000000 + 4 };
    static constexpr uint64_t blip_l4_s2 = { 0b1100000000000000110000000000000011000000000000001100000000000000 + 4 };
    static constexpr uint64_t blip_l4_s3 = { 0b1110000000000000111100000000000011110000000000001110000000000000 + 4 };
    static constexpr uint64_t blip_l4_s4 = { 0b1111110000000000111111110000000011111111000000001111110000000000 + 4 };

    // continuous signals                   7654321|7654321|7654321|7654321|7654321|7654321|7654321|76543000 + len
    static constexpr uint64_t cont_s1 = { 0b1000000000000000100000000000000010000000000000001000000000000000 + 0 };
    static constexpr uint64_t cont_s2 = { 0b1100000000000000110000000000000011000000000000001100000000000000 + 0 };

    // clang-format on
  }

  class LRAHandler : public Task::Task
  {
   private:
    IOpins::IOpin& m_lraActivityLED;
    unsigned       m_slice { 0 };
    uint64_t       m_pattern { 0 };
    uint64_t       m_patternReload { 0 };
    unsigned       buttons { 0 };

    inline void startPattern(uint64_t const pattern)
    {
      m_slice   = (pattern & 0b111) * 16;
      m_pattern = pattern & ~0b111;
      if (m_slice != 0)  // non-repetitive
        m_patternReload = 0;
      else
        m_patternReload = m_pattern;
      pinLRA_nENABLE = !0;
    };

   public:
    constexpr LRAHandler(uint32_t const delay, uint32_t const period, IOpins::IOpin& lraActivityLED)
        : Task(delay, period)
        , m_lraActivityLED(lraActivityLED) {};

    inline void body(void)
    {
      unsigned newButtons = IOpins::readButtons();
      if (buttons == 0 && newButtons == 0b0001)
        startPattern(LRA::blip_l4_s4);
      else if (buttons == 0 && newButtons == 0b0010)
        startPattern(LRA::cont_s1);
      else if (buttons == 0 && newButtons == 0b0100)
        startPattern(LRA::off);
      buttons = newButtons;

      if (m_slice == 0 && m_patternReload != 0)
      {
        m_slice   = 64;
        m_pattern = m_patternReload;
      }

      if (m_slice)
      {
        m_slice--;
        if (m_slice)
        {
          pinLRA_nENABLE = !1;
          pinLRA_0       = (m_pattern & (uint64_t(1) << 63)) != 0;
          m_pattern <<= 1;
        }
        else
          pinLRA_nENABLE = !0;
      }
      else
        pinLRA_nENABLE = !0;
    }
  };

}
