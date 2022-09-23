#pragma once

#include <stdint.h>
#include "cmsis/LPC43xx.h"
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

enum
{  // ADC data block
  BLOCK0 = 0x00,
  BLOCK1 = 0x08,
  BLOCK2 = 0x10,
  BLOCK3 = 0x18,
};

static __attribute__((always_inline)) inline void muxSelect(unsigned const group, unsigned const channel)
{
  if (group == GROUP0)
  {
    pinADC_MG0_A0 = channel & 0b01;
    pinADC_MG0_A1 = channel & 0b10;
  }
  else
  {
    pinADC_MG1_A0 = channel & 0b01;
    pinADC_MG1_A1 = channel & 0b10;
  }
}

#define ADC_START (((3 - 1) << ADC0_CR_CLKDIV_Pos) /* 12MHz / 3 --> 4MHz */                                     \
                   | (0 << ADC0_CR_BURST_Pos)      /* 0: manual software triggered mode */                      \
                   | (0 << ADC0_CR_CLKS_Pos)       /* "11" clocks --> 10 bits resolution, 363kHz sample rate */ \
                   | (1 << ADC0_CR_PDN_Pos)        /* enable ADC */                                             \
                   | (1 << ADC0_CR_START_Pos))     /* start = 1 required for manual mode */

static __attribute__((always_inline)) inline void startADC(unsigned const channel0, unsigned const channel1)
{
  LPC_ADC0->CR = ((1u << channel0) << ADC0_CR_SEL_Pos) | ADC_START;
  LPC_ADC1->CR = ((1u << channel1) << ADC1_CR_SEL_Pos) | ADC_START;
}

//
// ---- Get 4 samples total per ADC, but use only the last one.
//      This allows longer settling time for external muxer and better stability/crosstalk
//
static __attribute__((always_inline)) inline void waitAndFetchADCs(unsigned* const pAdc0, unsigned* const pAdc1)
{
  for (unsigned i = 0; i < 3; i++)
  {
    while ((LPC_ADC0->GDR & ADC0_GDR_DONE_Msk) == 0)
      ;
    while ((LPC_ADC1->GDR & ADC1_GDR_DONE_Msk) == 0)
      ;
    // restart ADCs
    LPC_ADC0->CR |= (1 << ADC0_CR_START_Pos);
    LPC_ADC1->CR |= (1 << ADC0_CR_START_Pos);
  }

  while ((LPC_ADC0->GDR & ADC0_GDR_DONE_Msk) == 0)
    ;
  while ((LPC_ADC1->GDR & ADC1_GDR_DONE_Msk) == 0)
    ;

  // fetch values
  (*pAdc0) = (LPC_ADC0->GDR & ADC0_GDR_V_VREF_Msk) >> ADC0_GDR_V_VREF_Pos;
  (*pAdc1) = (LPC_ADC1->GDR & ADC1_GDR_V_VREF_Msk) >> ADC1_GDR_V_VREF_Pos;
}

static __attribute__((noinline)) void adcCycle(unsigned const outChannel)
{
  unsigned adc0;
  unsigned adc1;
  // cycle 0, using muxer group 0
  startADC(ACH0, ACH1);

  waitAndFetchADCs(&adc0, &adc1);

  IPC_WriteAdcBuffer(outChannel + ACH0, adc0);
  IPC_WriteAdcBuffer(outChannel + ACH1, adc1);
  startADC(ACH2, ACH3);

  waitAndFetchADCs(&adc0, &adc1);

  muxSelect(GROUP0, (outChannel >> 3) + 1);  // prepare muxer group 0 for cycle 0
  IPC_WriteAdcBuffer(outChannel + ACH2, adc0);
  IPC_WriteAdcBuffer(outChannel + ACH3, adc1);

  // cycle 1, using muxer group 1
  startADC(ACH4, ACH5);

  waitAndFetchADCs(&adc0, &adc1);

  IPC_WriteAdcBuffer(outChannel + ACH4, adc0);
  IPC_WriteAdcBuffer(outChannel + ACH5, adc1);
  startADC(ACH6, ACH7);

  waitAndFetchADCs(&adc0, &adc1);

  muxSelect(GROUP1, (outChannel >> 3) + 1);  // prepare muxer group 1 for cycle 1
  IPC_WriteAdcBuffer(outChannel + ACH6, adc0);
  IPC_WriteAdcBuffer(outChannel + ACH7, adc1);
}

static __attribute__((noinline)) void processADCs(void)
{
  // we rely on a faster conversion than the consumer actually requires.
  // atm, free-wheeling conversion rate ~2.3kHz, required is > 2.0kHz
  // so we have about 15%, or 80us, of safety margin
  if (!s.adcIsConverting)
    return;
  adcCycle(BLOCK0);  // CH00..CH07 : ERP0_W0..ERP3_W1
  adcCycle(BLOCK1);  // CH08..CH15 : ERP4_W0..ERP7_W1
  adcCycle(BLOCK2);  // CH16..CH23 : EHC0_C0..EHC3_C1
  adcCycle(BLOCK3);  // CH24..CH31 : Aftertouch, Bender, Ribbon0+1, AmbLight0+1, Voltage0+1

  // now, all adc channels have been read ==> sync read index to last write index
  IPC_AdcUpdateReadIndex();
  // Starting a new round of adc channel value read-ins, advance ipc write index first
  IPC_AdcBufferWriteNext();
  pinADC_IS_CONVERTING = s.adcIsConverting = 0;
}
