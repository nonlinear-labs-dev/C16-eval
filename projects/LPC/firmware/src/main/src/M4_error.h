#pragma once

#include "io/pins.h"

#define M4_LED_ERROR_PATTERN_GENERAL              (0b10110000000000000000000000000000)
#define M4_LED_ERROR_PATTERN_MAIN_DID_RETURN      M4_LED_ERROR_PATTERN_GENERAL
#define M4_LED_ERROR_PATTERN_CGU_PLL_DID_NOT_LOCK M4_LED_ERROR_PATTERN_GENERAL
#define M4_LED_ERROR_HARD_FAULTS                  (0b10110000000000000000000000000001)
#define M4_LED_ERROR_CPP_EXCEPTION                (0b10110000000000000000000000000010)
#define M4_LED_ERROR_TASK_OVERRUN                 (0b10110000000000000000000000000011)

static inline __attribute__((always_inline)) void M4_error(unsigned const ledErrorPattern)
{
  __asm volatile("cpsid i");
  PINS_CriticalPinsInit();
  while (1)
  {
    pinLED_ERROR   = ledErrorPattern & 0b10000000000000000000000000000000;
    pinLED_WARNING = ledErrorPattern & 0b01000000000000000000000000000000;
    pinLED_M4HB    = ledErrorPattern & 0b00100000000000000000000000000000;
    pinLED_M0HB    = ledErrorPattern & 0b00010000000000000000000000000000;

    pinLED_E = ledErrorPattern & 0b10000000;
    pinLED_F = ledErrorPattern & 0b01000000;
    pinLED_G = ledErrorPattern & 0b00100000;
    pinLED_H = ledErrorPattern & 0b00010000;
    pinLED_I = ledErrorPattern & 0b00001000;
    pinLED_J = ledErrorPattern & 0b00000100;
    pinLED_K = ledErrorPattern & 0b00000010;
    pinLED_L = ledErrorPattern & 0b00000001;
  }
}
