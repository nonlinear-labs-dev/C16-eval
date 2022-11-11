#pragma once

#include <stdint.h>

static inline constexpr uint32_t usToTicks(uint32_t const us)
{
  return (us + 75) / 125u;
};

static inline constexpr uint32_t msToTicks(uint32_t const ms)
{
  return (1000u * ms) / 125u;
};

extern volatile uint64_t ticker;
