#pragma once

#include <stdint.h>

namespace LRA
{

  enum class LraPatternsIds
  {
    Off = 0,

    SingleBlip_VerySoft,
    SingleBlip_Soft,
    SingleBlip_Medium,
    SingleBlip_Strong,

    DoubleBlip_VerySoft,
    DoubleBlip_Soft,
    DoubleBlip_Medium,
    DoubleBlip_Strong,

    QuadrupleBlip_VerySoft,
    QuadrupleBlip_Soft,
    QuadrupleBlip_Medium,
    QuadrupleBlip_Strong,

    Continuous_VerySoft,
    Continuous_Soft,
  };

}  // namespace
