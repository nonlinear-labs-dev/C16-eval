#pragma once

#include <stdint.h>
#include "io/pins.h"
#include "drv/LRApatternIds.h"

namespace LraHardware
{

  static constexpr unsigned resonanceFrequency            = 165ul;
  static constexpr unsigned slicesPerPeriod               = 16ul;  // 8 slices per half-cycle to allow for different strengths
  static constexpr unsigned resonancePeriodInMicroseconds = 1000000ul / resonanceFrequency / slicesPerPeriod;

  // clang-format off

  // pattern arrays (are read out MSB first)
  // lowest three bits are the sequence length in periods, 1..7, 0 is special and means infinite repeat

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

  typedef struct
  {
	enum LRA::LraPatternsIds patternId;
    uint64_t                 pattern;
  } tPatternLookup;

  static const tPatternLookup patternLookupTable[] =
  {
	{  LRA::LraPatternsIds::Off,                 off },

	{ LRA::LraPatternsIds::SingleBlip_VerySoft,     blip_l1_s1 },
	{ LRA::LraPatternsIds::SingleBlip_Soft,         blip_l1_s2 },
	{ LRA::LraPatternsIds::SingleBlip_Medium,       blip_l1_s3 },
	{ LRA::LraPatternsIds::SingleBlip_Strong,       blip_l1_s4 },

	{ LRA::LraPatternsIds::DoubleBlip_VerySoft,     blip_l2_s1 },
	{ LRA::LraPatternsIds::DoubleBlip_Soft,         blip_l2_s2 },
	{ LRA::LraPatternsIds::DoubleBlip_Medium,       blip_l2_s3 },
	{ LRA::LraPatternsIds::DoubleBlip_Strong,       blip_l2_s4 },

	{ LRA::LraPatternsIds::QuadrupleBlip_VerySoft,  blip_l4_s1 },
	{ LRA::LraPatternsIds::QuadrupleBlip_Soft,      blip_l4_s2 },
	{ LRA::LraPatternsIds::QuadrupleBlip_Medium,    blip_l4_s3 },
	{ LRA::LraPatternsIds::QuadrupleBlip_Strong,    blip_l4_s4 },

	{ LRA::LraPatternsIds::Continuous_VerySoft,     cont_s1 },
	{ LRA::LraPatternsIds::Continuous_Soft,         cont_s2 },
  };

  // clang-format on

  class LRA
  {
   private:
    pIO32    m_pLraPin;
    unsigned m_slice { 0 };
    uint64_t m_pattern { 0 };
    uint64_t m_patternReload { 0 };
    unsigned buttons { 0 };

   public:
    constexpr LRA(pIO32 pLraPin)
        : m_pLraPin(pLraPin) {};

    inline void startPattern(uint64_t const pattern)
    {
      if (pattern != off)
      {
        m_slice   = (pattern & 0b111) * 16;
        m_pattern = pattern & ~0b111;
        if (m_slice != 0)  // non-repetitive
          m_patternReload = 0;
        else
          m_patternReload = m_pattern;
        pinLRA_nENABLE = !0;
        (*m_pLraPin)   = 0;
      }
      else
        reset();
    };

    inline void reset(void)
    {
      (*m_pLraPin) = 0;
      m_slice      = 0;
      m_pattern = m_patternReload = 0;
    }

    int process(void)
    {
      int activity = 0;
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
          activity     = 1;
          (*m_pLraPin) = (0 != (m_pattern & (uint64_t(1) << 63)));
          m_pattern <<= 1;
        }
      }
      return activity;
    };
  };

}  // namespace
