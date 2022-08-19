#pragma once

#include "pin_setup.h"

#define LED_A GPIO_Word(2, 10)
#define LED_B GPIO_Word(0, 11)
#define LED_C GPIO_Word(1, 9)
#define LED_D GPIO_Word(1, 1)

static inline void PINS_Init(void)
{
  // (LED.A, green)
  GPIO_DIR_OUT(2, 10);
  SFSP(5, 1) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

  // (LED.B, green)
  GPIO_DIR_OUT(0, 11);
  SFSP(1, 4) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

  // (LED.C, orange)
  GPIO_DIR_OUT(1, 9);
  SFSP(1, 6) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

  // (LED.D, red)
  GPIO_DIR_OUT(1, 1);
  SFSP(1, 8) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
}
