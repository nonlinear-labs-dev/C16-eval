#pragma once

#include <stdint.h>
#include "cmsis/LPC43xx.h"
#include "cmsis/core_cmFunc.h"
#include "cmsis/core_cmInstr.h"
#include "ipc/ipc.h"
#include "io/pins.h"

enum
{  // ADCs
  ADC0 = 0,
  ADC1 = 1,
};

enum
{              // Muxer Groups
  GROUP0 = 0,  // M0..M3
  GROUP1 = 1,  // M4..M7
};

enum
{  // Muxer Input Channels
  MCH0 = 0,
  MCH1 = 1,
  MCH2 = 2,
  MCH3 = 3,
};

enum
{  // ADC Input Channels
  ACH0 = 0,
  ACH1 = 1,
  ACH2 = 2,
  ACH3 = 3,
  ACH4 = 4,
  ACH5 = 5,
  ACH6 = 6,
  ACH7 = 7,
};

static __attribute__((always_inline)) inline void muxSelect(unsigned const group, unsigned const channel)
{
  if (group == GROUP0)
  {
    ADC_MG0_A0 = channel & 0b01;
    ADC_MG0_A1 = channel & 0b10;
  }
  else
  {
    ADC_MG1_A0 = channel & 0b01;
    ADC_MG1_A1 = channel & 0b10;
  }
}

#define ADC_START (((3 - 1) << ADC0_CR_CLKDIV_Pos) /* 12MHz / 3 --> 4MHz */                                     \
                   | (0 << ADC0_CR_BURST_Pos)      /* 0: manual software triggered mode */                      \
                   | (0 << ADC0_CR_CLKS_Pos)       /* "11" clocks --> 10 bits resolution, 363kHz sample rate */ \
                   | (1 << ADC0_CR_PDN_Pos)        /* enable ADC */                                             \
                   | (1 << ADC0_CR_START_Pos))     /* start = 1 required for manual mode */

static __attribute__((always_inline)) inline void startADC(unsigned const adc, unsigned const channel)
{
  if (adc)
    LPC_ADC1->CR = ((1u << channel) << ADC0_CR_SEL_Pos) | ADC_START;
  else
    LPC_ADC0->CR = ((1u << channel) << ADC0_CR_SEL_Pos) | ADC_START;
}

static __attribute__((always_inline)) inline unsigned waitAndFetchADC(unsigned const adc)
{
  if (adc)
  {
    while ((LPC_ADC1->GDR & ADC0_GDR_DONE_Msk) == 0)
      ;
    return (LPC_ADC1->GDR & ADC0_GDR_V_VREF_Msk) >> ADC0_GDR_V_VREF_Pos;
  }
  else
  {
    while ((LPC_ADC0->GDR & ADC0_GDR_DONE_Msk) == 0)
      ;
    return (LPC_ADC0->GDR & ADC0_GDR_V_VREF_Msk) >> ADC0_GDR_V_VREF_Pos;
  }
}

static __attribute__((always_inline)) inline void adcCycle(unsigned const outChannel, unsigned const nextMuxChannel)
{
  // cycle 0, using muxer group 0
  startADC(ADC0, ACH0);
  startADC(ADC1, ACH1);
  __enable_irq();

  unsigned adc0 = waitAndFetchADC(0);
  unsigned adc1 = waitAndFetchADC(1);

  __disable_irq();
  IPC_WriteAdcBuffer(outChannel + ACH0, adc0);
  IPC_WriteAdcBuffer(outChannel + ACH1, adc1);
  startADC(ADC0, ACH2);
  startADC(ADC1, ACH3);
  __enable_irq();
  // have some spare time here

  adc0 = waitAndFetchADC(0);
  adc1 = waitAndFetchADC(1);
  muxSelect(GROUP0, (nextMuxChannel + 1) & 0b11);  // prepare muxer group 0 for cycle 0

  __disable_irq();
  IPC_WriteAdcBuffer(outChannel + ACH2, adc0);
  IPC_WriteAdcBuffer(outChannel + ACH3, adc1);

  // cycle 1, using muxer group 1
  startADC(ADC0, ACH4);
  startADC(ADC1, ACH5);
  __enable_irq();
  // have some spare time here

  adc0 = waitAndFetchADC(0);
  adc1 = waitAndFetchADC(1);

  __disable_irq();
  IPC_WriteAdcBuffer(outChannel + ACH4, adc0);
  IPC_WriteAdcBuffer(outChannel + ACH5, adc1);
  startADC(ADC0, ACH6);
  startADC(ADC1, ACH7);
  __enable_irq();
  // have some spare time here

  adc0 = waitAndFetchADC(0);
  adc1 = waitAndFetchADC(1);
  muxSelect(GROUP1, (nextMuxChannel + 2) & 0b11);  // prepare muxer group 1 for cycle 1

  __disable_irq();
  IPC_WriteAdcBuffer(outChannel + ACH6, adc0);
  IPC_WriteAdcBuffer(outChannel + ACH7, adc1);
}

static __attribute__((always_inline)) inline void processADCs(void)
{
  adcCycle(0x00, MCH0);  // CH00..CH07 : ERP0_W0..ERP3_W1
  adcCycle(0x08, MCH1);  // CH08..CH15 : ERP4_W0..ERP7_W1
  adcCycle(0x10, MCH2);  // CH16..CH23 : EHC0_C0..EHC3_C1
  adcCycle(0x18, MCH3);  // CH24..CH31 : Aftertouch, Bender, Ribbon0+1, AmbLight0+1, Voltage0+1

  DBG_TP1_0 = ~DBG_TP1_0;

  // now, all adc channels have been read ==> sync read index to last write index
  IPC_AdcUpdateReadIndex();
  // Starting a new round of adc channel value read-ins, advance ipc write index first
  IPC_AdcBufferWriteNext();
}
