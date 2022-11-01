#pragma once

#include <stdint.h>
#include "io/pins.h"

namespace EhcHardware
{

  inline void configure(uint8_t const seriesRs, uint8_t const pullupRs, uint8_t const pulldownRs)
  {
    pinEHC_nSER_0 = !(seriesRs & (1 << 0));
    pinEHC_nSER_1 = !(seriesRs & (1 << 1));
    pinEHC_nSER_2 = !(seriesRs & (1 << 2));
    pinEHC_nSER_3 = !(seriesRs & (1 << 3));
    pinEHC_nSER_4 = !(seriesRs & (1 << 4));
    pinEHC_nSER_5 = !(seriesRs & (1 << 5));
    pinEHC_nSER_6 = !(seriesRs & (1 << 6));
    pinEHC_nSER_7 = !(seriesRs & (1 << 7));

    pinEHC_PU_0 = (pullupRs & (1 << 0));
    pinEHC_PU_1 = (pullupRs & (1 << 1));
    pinEHC_PU_2 = (pullupRs & (1 << 2));
    pinEHC_PU_3 = (pullupRs & (1 << 3));
    pinEHC_PU_4 = (pullupRs & (1 << 4));
    pinEHC_PU_5 = (pullupRs & (1 << 5));
    pinEHC_PU_6 = (pullupRs & (1 << 6));
    pinEHC_PU_7 = (pullupRs & (1 << 7));

    pinEHC_PD_0 = (pulldownRs & (1 << 0));
    pinEHC_PD_1 = (pulldownRs & (1 << 1));
    pinEHC_PD_2 = (pulldownRs & (1 << 2));
    pinEHC_PD_3 = (pulldownRs & (1 << 3));
    pinEHC_PD_4 = (pulldownRs & (1 << 4));
    pinEHC_PD_5 = (pulldownRs & (1 << 5));
    pinEHC_PD_6 = (pulldownRs & (1 << 6));
    pinEHC_PD_7 = (pulldownRs & (1 << 7));
  }

  //
  //  #define ehcSetup_Default(channel)  pinEHC_PU_##channel = 1, pinEHC_PD_##channel = 1, pinEHC_nSER_##channel = !1
  //  #define ehcSetup_Unloaded(channel) pinEHC_PU_##channel = 0, pinEHC_PD_##channel = 0, pinEHC_nSER_##channel = !1
  //  #define ehcSetup_Pullup(channel)   pinEHC_PU_##channel = 1, pinEHC_PD_##channel = 0, pinEHC_nSER_##channel = !0
  //  #define ehcSetup_CVm5p5(channel)   pinEHC_PU_##channel = 1, pinEHC_PD_##channel = 0, pinEHC_nSER_##channel = !1
  //  #define ehcSetup_CV0p10(channel)   pinEHC_PU_##channel = 0, pinEHC_PD_##channel = 1, pinEHC_nSER_##channel = !1
  //

}  // namespace
