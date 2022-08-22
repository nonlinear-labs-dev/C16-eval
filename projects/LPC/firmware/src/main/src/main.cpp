/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC43xx.h"
#endif

#include <cr_section_macros.h>

#if defined(__MULTICORE_MASTER_SLAVE_M0APP) | defined(__MULTICORE_MASTER_SLAVE_M0SUB)
#include "cr_start_m0.h"
#endif

// TODO: insert other include files here
#include "io/pins.h"

// TODO: insert other definitions and declarations here

int main(void)
{
  LED_WARNING = LED_ERROR = 0;

  // Start M0APP slave processor
#if defined(__MULTICORE_MASTER_SLAVE_M0APP)
  cr_start_m0(SLAVE_M0APP, &__core_m0app_START__);
#endif

  // Start M0SUB slave processor
#if defined(__MULTICORE_MASTER_SLAVE_M0SUB)
//    cr_start_m0(SLAVE_M0SUB,&__core_m0sub_START__);
#endif

  // TODO: insert code here

  // Force the counter to be placed into memory
  static volatile unsigned i = 0;
  // Enter an infinite loop, just incrementing a counter
  while (1)
  {
    if (i > 300000lu)
      i = 0;
    if (i == 0)
    {
      LED_M4HB = ~LED_M4HB;
    }
    i++;
  }
  return 0;
}
