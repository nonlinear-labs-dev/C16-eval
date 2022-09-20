#pragma once

#include <stdint.h>
#include "pin_setup.h"

static uint32_t DUMMY_PIN;

//
// define pins and set default safe states
//
static inline void PINS_CriticalPinsInit(void);
static inline void PINS_NormalPinsInit(void);

static inline void PINS_Init(void)
{
  PINS_CriticalPinsInit();
  PINS_NormalPinsInit();
}

static inline void PINS_NormalPinsInit(void)
{
  DUMMY_PIN = 0;

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
  // LED_E (TP2_4) Yellow
#define LED_E GPIO_Word(5, 4)
  GPIO_DIR_OUT(5, 4);
  SFSP(2, 4) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  LED_E      = 0u;

  // LED_F (TP2_3) Yellow
#define LED_F GPIO_Word(5, 3)
  GPIO_DIR_OUT(5, 3);
  SFSP(2, 3) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  LED_F      = 0u;

  // LED_G (TP6_8) Yellow
#define LED_G GPIO_Word(5, 16)
  GPIO_DIR_OUT(5, 16);
  SFSP(6, 8) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  LED_G      = 0u;

  // LED_H (TP6_7) Yellow
#define LED_H GPIO_Word(5, 15)
  GPIO_DIR_OUT(5, 15);
  SFSP(6, 7) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  LED_H      = 0u;

  // LED_I (TP2_2) Red
#define LED_I GPIO_Word(5, 2)
  GPIO_DIR_OUT(5, 2);
  SFSP(2, 2) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  LED_I      = 0u;

  // LED_J (TP6_6) Red
#define LED_J GPIO_Word(0, 5)
  GPIO_DIR_OUT(0, 5);
  SFSP(6, 6) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LED_J      = 0u;

  // LED_K (TP6_5) Red
#define LED_K GPIO_Word(3, 4)
  GPIO_DIR_OUT(3, 4);
  SFSP(6, 5) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LED_K      = 0u;

  // LED_L (TP6_4) Red
#define LED_L GPIO_Word(3, 3)
  GPIO_DIR_OUT(3, 3);
  SFSP(6, 4) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LED_L      = 0u;

  //
  // ---- EHC range control
  //
  // EHC_PU_0
#define EHC_PU_0 GPIO_Word(1, 3)
  GPIO_DIR_OUT(1, 3);
  SFSP(1, 10) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PU_0    = 1u;

  // EHC_PU_1
#define EHC_PU_1 GPIO_Word(1, 2)
  GPIO_DIR_OUT(1, 2);
  SFSP(1, 9) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PU_1   = 1u;

  // EHC_PU_2
#define EHC_PU_2 GPIO_Word(5, 10)
  GPIO_DIR_OUT(5, 10);
  SFSP(3, 7) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  EHC_PU_2   = 1u;

  // EHC_PU_3
#define EHC_PU_3 GPIO_Word(3, 12)
  GPIO_DIR_OUT(3, 12);
  SFSP(7, 4) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PU_3   = 1u;

  // EHC_PU_4
#define EHC_PU_4 GPIO_Word(3, 8)
  GPIO_DIR_OUT(3, 8);
  SFSP(7, 0) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PU_4   = 1u;

  // EHC_PU_5
#define EHC_PU_5 GPIO_Word(3, 10)
  GPIO_DIR_OUT(3, 10);
  SFSP(6, 2) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PU_5   = 1u;

  // EHC_PU_6
#define EHC_PU_6 GPIO_Word(3, 5)
  GPIO_DIR_OUT(3, 5);
  SFSP(6, 9) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PU_6   = 1u;

  // EHC_PU_7
#define EHC_PU_7 GPIO_Word(2, 8)
  GPIO_DIR_OUT(2, 8);
  SFSP(6, 12) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PU_7    = 1u;

  // EHC_PD_0
#define EHC_PD_0 GPIO_Word(1, 4)
  GPIO_DIR_OUT(1, 4);
  SFSP(1, 11) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PD_0    = 1u;

  // EHC_PD_1
#define EHC_PD_1 GPIO_Word(1, 0)
  GPIO_DIR_OUT(1, 0);
  SFSP(1, 7) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PD_1   = 1u;

  // EHC_PD_2
#define EHC_PD_2 GPIO_Word(3, 13)
  GPIO_DIR_OUT(3, 13);
  SFSP(7, 5) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PD_2   = 1u;

  // EHC_PD_3
#define EHC_PD_3 GPIO_Word(3, 12)
  GPIO_DIR_OUT(3, 12);
  SFSP(7, 4) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PD_3   = 1u;

  // EHC_PD_4
#define EHC_PD_4 GPIO_Word(3, 8)
  GPIO_DIR_OUT(3, 8);
  SFSP(7, 0) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PD_4   = 1u;

  // EHC_PD_5
#define EHC_PD_5 GPIO_Word(3, 9)
  GPIO_DIR_OUT(3, 9);
  SFSP(7, 1) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PD_5   = 1u;

  // EHC_PD_6
#define EHC_PD_6 GPIO_Word(3, 7)
  GPIO_DIR_OUT(3, 7);
  SFSP(6, 11) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PD_6    = 1u;

  // EHC_PD_7
#define EHC_PD_7 GPIO_Word(3, 6)
  GPIO_DIR_OUT(3, 6);
  SFSP(6, 10) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_PD_7    = 1u;

  // EHC_DET_0
#define EHC_DET_0 GPIO_Word(1, 5)
  GPIO_DIR_IN(1, 5);
  SFSP(1, 12) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

  // EHC_DET_1
#define EHC_DET_1 GPIO_Word(1, 14)
  GPIO_DIR_IN(1, 14);
  SFSP(3, 4) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

  // EHC_DET_2
#define EHC_DET_2 GPIO_Word(1, 11)
  GPIO_DIR_IN(1, 11);
  SFSP(2, 11) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;

  // EHC_DET_3
#define EHC_DET_3 GPIO_Word(5, 5)
  GPIO_DIR_IN(5, 5);
  SFSP(2, 5) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
}

static inline void PINS_CriticalPinsInit(void)
{
  //
  // ---- LRA actuators (shakers)
  //

  // LRA_nENABLE
#define LRA_nENABLE GPIO_Word(0, 12)
  GPIO_DIR_OUT(0, 12);
  SFSP(1, 17) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LRA_nENABLE = 0u;

  // LRA_0
#define LRA_0 GPIO_Word(0, 15)
  GPIO_DIR_OUT(0, 15);
  SFSP(1, 20) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LRA_0       = 0u;

  // LRA_1
#define LRA_1 GPIO_Word(4, 11)
  GPIO_DIR_OUT(4, 11);
  SFSP(9, 6) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LRA_1      = 0u;

  // LRA_2
#define LRA_2 GPIO_Word(5, 18)
  GPIO_DIR_OUT(5, 18);
  SFSP(9, 5) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  LRA_2      = 0u;

  // LRA_3
#define LRA_3 GPIO_Word(0, 13)
  GPIO_DIR_OUT(0, 13);
  SFSP(1, 18) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LRA_3       = 0u;

  // LRA_4
#define LRA_4 GPIO_Word(2, 15)
  GPIO_DIR_OUT(2, 15);
  SFSP(5, 6) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LRA_4      = 0u;

  // LRA_5
#define LRA_5 GPIO_Word(1, 7)
  GPIO_DIR_OUT(1, 7);
  SFSP(1, 14) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LRA_5       = 0u;

  // LRA_6
#define LRA_6 GPIO_Word(2, 14)
  GPIO_DIR_OUT(2, 14);
  SFSP(5, 5) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LRA_6      = 0u;

  // LRA_7
#define LRA_7 GPIO_Word(1, 6)
  GPIO_DIR_OUT(1, 6);
  SFSP(1, 13) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  LRA_7       = 0u;

  //
  // ---- EHC range control
  //
  // EHC_nSER_0
#define EHC_nSER_0 GPIO_Word(2, 12)
  GPIO_DIR_OUT(2, 12);
  SFSP(5, 3) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_nSER_0 = 0u;

  // EHC_nSER_1
#define EHC_nSER_1 GPIO_Word(2, 13)
  GPIO_DIR_OUT(2, 13);
  SFSP(5, 4) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_nSER_1 = 0u;

  // EHC_nSER_2
#define EHC_nSER_2 GPIO_Word(1, 15)
  GPIO_DIR_OUT(1, 15);
  SFSP(3, 5) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_nSER_2 = 0u;

  // EHC_nSER_3
#define EHC_nSER_3 GPIO_Word(0, 6)
  GPIO_DIR_OUT(0, 6);
  SFSP(3, 6) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_nSER_3 = 0u;

  // EHC_nSER_4
#define EHC_nSER_4 GPIO_Word(1, 12)
  GPIO_DIR_OUT(1, 12);
  SFSP(2, 12) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_nSER_4  = 0u;

  // EHC_nSER_5
#define EHC_nSER_5 GPIO_Word(1, 13)
  GPIO_DIR_OUT(1, 13);
  SFSP(2, 13) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_nSER_5  = 0u;

  // EHC_nSER_6
#define EHC_nSER_6 GPIO_Word(5, 6)
  GPIO_DIR_OUT(5, 6);
  SFSP(2, 6) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 4;
  EHC_nSER_6 = 0u;

  // EHC_nSER_7
#define EHC_nSER_7 GPIO_Word(0, 14)
  GPIO_DIR_OUT(0, 14);
  SFSP(2, 10) = SFS_EIF + SFS_EIB + SFS_DHS + SFS_EPD + SFS_EPU + 0;
  EHC_nSER_7  = 0u;
}

static inline void PINS_AllLedsOn(void)
{
  LED_M0HB = LED_M4HB = LED_WARNING = LED_ERROR = 1u;
}

#ifdef __cplusplus
 static constexpr
#endif
