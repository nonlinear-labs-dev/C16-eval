/*
===============================================================================
 Name        : co-proc.c
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

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

#if defined(M0_SLAVE_PAUSE_AT_MAIN)
volatile unsigned int pause_at_main;
#endif

int main(void)
{

#if defined(M0_SLAVE_PAUSE_AT_MAIN)
  // Pause execution until debugger attaches and modifies variable
  while (pause_at_main == 0)
  {
  }
#endif

  // TODO: insert code here

  // Force the counter to be placed into memory
  static volatile int i = 0;
  // Enter an infinite loop, just incrementing a counter
  while (1)
  {
    i++;
  }
  return 0;
}
