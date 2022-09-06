#pragma once

#include "pin_setup.h"

//
// define pins and set default safe states
//
static inline void PINS_Init(void)
{
  //
  // -------- System LEDs --------
  //
  // (LED.A, green)
#define LED_M0HB GPIO_Word(2, 10)
  GPIO_DIR_OUT(2, 10);
  SFSP(5, 1) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LED_M0HB   = 1u;

  // (LED.B, green)
#define LED_M4HB GPIO_Word(0, 11)
  GPIO_DIR_OUT(0, 11);
  SFSP(1, 4) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LED_M4HB   = 1u;

  // (LED.C, orange)
#define LED_WARNING GPIO_Word(1, 9)
  GPIO_DIR_OUT(1, 9);
  SFSP(1, 6)  = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LED_WARNING = 1u;

  // (LED.ERROR, red)
#define LED_ERROR GPIO_Word(1, 1)
  GPIO_DIR_OUT(1, 1);
  SFSP(1, 8) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LED_ERROR  = 1u;

  //
  // -------- KeybedScanner --------
  //
#define KS_LINE_0 GPIO_Word(0, 0)
  GPIO_DIR_OUT(0, 0);
  SFSP(0, 0) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  KS_LINE_0  = 0u;

#define KS_LINE_1 GPIO_Word(0, 1)
  GPIO_DIR_OUT(0, 1);
  SFSP(0, 1) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  KS_LINE_1  = 0u;

#define KS_LINE_2 GPIO_Word(0, 2)
  GPIO_DIR_OUT(0, 2);
  SFSP(1, 15) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  KS_LINE_2   = 0u;

#define KS_LINE_3 GPIO_Word(0, 3)
  GPIO_DIR_OUT(0, 3);
  SFSP(1, 16) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  KS_LINE_3   = 0u;

#define KS_KEY_0 GPIO_Word(2, 0)
  GPIO_DIR_IN(2, 0);
  SFSP(4, 0) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

#define KS_KEY_1 GPIO_Word(2, 1)
  GPIO_DIR_IN(2, 1);
  SFSP(4, 1) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

#define KS_KEY_2 GPIO_Word(2, 2)
  GPIO_DIR_IN(2, 2);
  SFSP(4, 2) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

#define KS_KEY_3 GPIO_Word(2, 3)
  GPIO_DIR_IN(2, 3);
  SFSP(4, 3) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

#define KS_KEY_4 GPIO_Word(2, 4)
  GPIO_DIR_IN(2, 4);
  SFSP(4, 4) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

#define KS_KEY_5 GPIO_Word(2, 5)
  GPIO_DIR_IN(2, 5);
  SFSP(4, 5) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

#define KS_KEY_6 GPIO_Word(2, 6)
  GPIO_DIR_IN(2, 6);
  SFSP(4, 6) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

#define KS_KEY_7 GPIO_Word(2, 7)
  GPIO_DIR_IN(2, 7);
  SFSP(5, 7) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

  //
  // -------- ADC Muxer Groups --------
  //
#define ADC_MG0_A0 GPIO_Word(3, 14)
  GPIO_DIR_OUT(3, 14);
  SFSP(7, 6) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  ADC_MG0_A0 = 0u;

#define ADC_MG0_A1 GPIO_Word(3, 15)
  GPIO_DIR_OUT(3, 15);
  SFSP(7, 7) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  ADC_MG0_A1 = 0u;

#define ADC_MG1_A0 GPIO_Word(5, 9)
  GPIO_DIR_OUT(5, 9);
  SFSP(3, 2) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  ADC_MG1_A0 = 0u;

#define ADC_MG1_A1 GPIO_Word(3, 11)
  GPIO_DIR_OUT(3, 11);
  SFSP(7, 3) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  ADC_MG1_A1 = 0u;

  //
  // -------- Debug Pins --------
  //
  // TP_1_0 : M0 ADC Cycle (toggles state after a complete cycle)
#define DBG_TP1_0 GPIO_Word(0, 4)
  GPIO_DIR_OUT(0, 4);
  SFSP(1, 0) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  DBG_TP1_0  = 0u;

  //
  // -------- Aux/Debug LEDs --------
  //
  // LED_E (TP2_4)
#define LED_E GPIO_Word(5, 4)
  GPIO_DIR_OUT(5, 4);
  SFSP(2, 4) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  LED_E      = 0u;

  // LED_F (TP2_3)
#define LED_F GPIO_Word(5, 3)
  GPIO_DIR_OUT(5, 3);
  SFSP(2, 3) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  LED_F      = 0u;

  // LED_G (TP6_8)
#define LED_G GPIO_Word(5, 16)
  GPIO_DIR_OUT(5, 16);
  SFSP(6, 8) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  LED_G      = 0u;

  // LED_H (TP6_7)
#define LED_H GPIO_Word(5, 15)
  GPIO_DIR_OUT(5, 15);
  SFSP(6, 7) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  LED_H      = 0u;

  // LED_I (TP2_2)
#define LED_I GPIO_Word(5, 2)
  GPIO_DIR_OUT(5, 2);
  SFSP(2, 2) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  LED_I      = 0u;

  // LED_J (TP6_6)
#define LED_J GPIO_Word(0, 5)
  GPIO_DIR_OUT(0, 5);
  SFSP(6, 6) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LED_J      = 0u;

  // LED_K (TP6_5)
#define LED_K GPIO_Word(3, 4)
  GPIO_DIR_OUT(3, 4);
  SFSP(6, 5) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LED_K      = 0u;

  // LED_L (TP6_4)
#define LED_L GPIO_Word(3, 3)
  GPIO_DIR_OUT(3, 3);
  SFSP(6, 4) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LED_L      = 0u;
}

static inline void PINS_AllLedsOn(void)
{
  LED_M0HB = LED_M4HB = LED_WARNING = LED_ERROR = 1u;
}
