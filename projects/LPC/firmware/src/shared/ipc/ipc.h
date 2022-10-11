#pragma once
#include <stdint.h>

#define LPC_DGB_ADC_STRESS_TEST (0)

#if LPC_DGB_ADC_STRESS_TEST
// use a free-running 0..1024 counter rather than real ADC values
// counter is incremented when adcBufferWriteIndex is advanced
#warning "ADC stress test is on, no real ADC values will be used"
static uint16_t adc_val;
#endif

//
//  -------- ADC --------
//
#define IPC_ADC_NUMBER_OF_CHANNELS (32u)
#define ADC_RESOLUTION             (1024u)

enum IPC_ADC_CHANNEL_IDS  // DO NOT CHANGE !
{
  // Endless Rotary Pots
  ADC_ERP0_W0 = 0,
  ADC_ERP0_W1 = 1,
  ADC_ERP1_W0 = 2,
  ADC_ERP1_W1 = 3,
  ADC_ERP2_W0 = 4,
  ADC_ERP2_W1 = 5,
  ADC_ERP3_W0 = 6,
  ADC_ERP3_W1 = 7,
  ADC_ERP4_W0 = 8,
  ADC_ERP4_W1 = 9,
  ADC_ERP5_W0 = 10,
  ADC_ERP5_W1 = 11,
  ADC_ERP6_W0 = 12,
  ADC_ERP6_W1 = 13,
  ADC_ERP7_W0 = 14,
  ADC_ERP7_W1 = 15,

  // External Hardware Controllers (Pedals)
  ADC_EHC0 = 16,
  ADC_EHC1 = 17,
  ADC_EHC2 = 18,
  ADC_EHC3 = 19,
  ADC_EHC4 = 20,
  ADC_EHC5 = 21,
  ADC_EHC6 = 22,
  ADC_EHC7 = 23,

  /* Internal Hardware Controllers */
  ADC_IHC0 = 24,  // Aftertouch
  ADC_AT   = ADC_IHC0,
  ADC_IHC1 = 25,  // PitchBender
  ADC_PB   = ADC_IHC1,
  ADC_IHC2 = 26,  // Ribbon1
  ADC_RIB1 = ADC_IHC2,
  ADC_IHC3 = 27,  // Ribbon2
  ADC_RIB2 = ADC_IHC3,

  /* miscellaneous */
  ADC_AMBLI1  = 28,  // Ambient Light Sensor 1
  ADC_AMBLI2  = 29,  // Ambient Light Sensor 2
  ADC_PSU_19V = 30,  // 19V Supply Monitor
  ADC_PSU_5V  = 31,  // 5V Supply Monitor
};

// ADC ring buffers
// Must be 2^N in size and >= 2. This also determines the averaging.
// Size should NOT be very much larger than the number of aquisitions between M4 read-out operations
#define IPC_ADC_BUFFER_SIZE (2u)
#define IPC_ADC_BUFFER_MASK (IPC_ADC_BUFFER_SIZE - 1)
#define IPC_ADC_DEFAULT     (ADC_RESOLUTION / 2)

#define ADC_VIRTUAL_RESOLUTION (ADC_RESOLUTION * IPC_ADC_BUFFER_SIZE)

typedef struct
{
  uint32_t values[IPC_ADC_NUMBER_OF_CHANNELS][IPC_ADC_BUFFER_SIZE];
  int32_t  sum[IPC_ADC_NUMBER_OF_CHANNELS];
} ADC_BUFFER_ARRAY_T;

//
//  -------- Keybed Scanner --------
//
#define IPC_KEYBUFFER_SIZE       (64u)  // number of key events that can be processed in 125us
#define IPC_KEYBUFFER_MASK       (IPC_KEYBUFFER_SIZE - 1u)
#define IPC_KEYBUFFER_KEYMASK    (0x3F)
#define IPC_KEYBUFFER_NOTEON     (0x40)
#define IPC_KEYBUFFER_TIME_SHIFT (7u)

//
// -------- Shared M0/M4-core Data Structure
//
typedef struct
{
  volatile uint32_t ticker5us;
  volatile uint32_t timesliceTicker5us;
  uint32_t          keyBufferData[IPC_KEYBUFFER_SIZE];
#ifdef CORE_M4
  volatile
#endif
      uint32_t       keyBufferWritePos;
  uint32_t           keyBufferReadPos;
  uint32_t           M0_KbsIrqOvers;
  ADC_BUFFER_ARRAY_T adcBufferData;
  uint32_t           adcBufferWriteIndex;
  uint32_t           adcBufferReadIndex;
  uint32_t           adcIsConverting;
} SharedData_T;

extern SharedData_T s;

/******************************************************************************/
/**	@brief  setup and clear data in shared memory
*******************************************************************************/
inline static void IPC_Init(void)
{
  s.ticker5us          = 0;
  s.timesliceTicker5us = 0;

  for (unsigned i = 0; i < IPC_KEYBUFFER_SIZE; i++)
    s.keyBufferData[i] = 0;

  s.keyBufferWritePos = 0;
  s.keyBufferReadPos  = 0;
  s.M0_KbsIrqOvers    = 0;

  for (unsigned i = 0; i < IPC_ADC_NUMBER_OF_CHANNELS; i++)
  {
    for (unsigned k = 0; k < IPC_ADC_BUFFER_SIZE; k++)
      s.adcBufferData.values[i][k] = IPC_ADC_DEFAULT;
    s.adcBufferData.sum[i] = IPC_ADC_DEFAULT * IPC_ADC_BUFFER_SIZE;
  }
  s.adcBufferReadIndex  = 0;
  s.adcBufferWriteIndex = 0;
  s.adcIsConverting     = 0;
}

/******************************************************************************
*	Functions for both the M4 and M0 to interface the PlayBuffers.
******************************************************************************/
/******************************************************************************/
/**  @brief     Here the M0 write key events to a circular buffer
                that the M4 will read
     @param[in] keyEvent: A struct containing the index of the key
                and the direction and travel time of the last key action
*******************************************************************************/
static inline void IPC_M0_KeyBuffer_WriteKeyEvent(uint32_t const keyEvent)
{
  // !! this is a potentially critical section !!
  // Emphase_IPC_M4_KeyBuffer_ReadBuffer() should not run while we are here
  // because it uses keyBufferWritePos in a compare. Unless keyBufferWritePos
  // isn't updated so fast as to overrun keyBufferReadPos nothing bad will happen, though.
  // NOTE : No check is done anyway if the write overruns the buffer, we rely
  // on the buffer being big enough to avoid this.
  s.keyBufferData[s.keyBufferWritePos] = keyEvent;
  s.keyBufferWritePos                  = (s.keyBufferWritePos + 1) & (IPC_KEYBUFFER_MASK);
}

/******************************************************************************/
/** @brief      Here the M4 reads key events from a circular buffer
                that have been written by the M0
    @return     key event (0: nothing to do)
*******************************************************************************/
static inline uint32_t IPC_M4_KeyBuffer_ReadBuffer(void)
{
  // !! this is a potentially critical section !!
  // Emphase_IPC_M0_KeyBuffer_WriteKeyEvent() should not run while we are here
  // because it updates keyBufferWritePos.
  // As long as keyBufferWritePos, which is volatile from M4's view, doesn't update so fast
  // that it overruns keyBufferReadPos (which is extremely unlikely, only when Emphase_IPC_M4_KeyBuffer_ReadBuffer
  // is interrupted/stalling for a really long time), nothing bad will happen, though.
  unsigned event = 0;
  if (s.keyBufferReadPos != s.keyBufferWritePos)
  {
    event              = s.keyBufferData[s.keyBufferReadPos];
    s.keyBufferReadPos = (s.keyBufferReadPos + 1) & (IPC_KEYBUFFER_MASK);
  }
  return event;
}

/******************************************************************************/
/** @brief      Return size of key buffer
*******************************************************************************/
static inline unsigned IPC_KeyBuffer_GetSize()
{
  return IPC_KEYBUFFER_SIZE;
}

//
//  -------- ADC --------
//

/******************************************************************************/
/**	@brief      Read ADC channel value as sum of whole ring buffer contents
*   @param[in]	IPC id of the adc channel 0...31
*   @return     adc channel value
******************************************************************************/
static inline uint32_t IPC_ReadAdcBufferSum(unsigned const adc_id)
{
  // M0 may advance adcBufferReadIndex while we are reading values for a number
  // of ADCs during an M4 time-slice. This means values of different ADCs may
  // not all be from the same conversion cycle of M0. Since the conversion cycle
  // runs several times per M4 time-slice the error is neglegible, though.
  // More importantly, M0 may be just in the middle of its non-atomic
  // read-modify-write operation in IPC_WriteAdcBuffer, but again the error
  // is very small because only a delta is added/subtracted.
  // The addition of 1/2 of the buffer(=averaging) size make the final
  // value range more symmetric, now spanning from 4 to 8192-4 rather than
  // from 0 to 8192-8 (which comes from 8*1023). Better round results should
  // a process decide it wants less bits of resolution.
  return ((uint32_t) s.adcBufferData.sum[adc_id] + IPC_ADC_BUFFER_SIZE / 2u);
}

/******************************************************************************/
/**	@brief      Write ADC value (must be called only once per cycle!)
*   @param[in]	IPC id of the adc channel 0...15
*   @param[in]  adc channel value
******************************************************************************/
#if LPC_DGB_ADC_STRESS_TEST
static inline void IPC_WriteAdcBuffer(unsigned const adc_id, uint32_t value)
{
  value = adc_val;
#else
static inline void IPC_WriteAdcBuffer(unsigned const adc_id, uint32_t const value)
{
#endif
  // see notes for IPC_ReadAdcBufferAveraged above.
  // Interrupts should be disabled.
  // subtract out the overwritten value and add in new value to sum
  int32_t const tmp = -((int32_t)(s.adcBufferData.values[adc_id][s.adcBufferWriteIndex])) + (int32_t) value;
  s.adcBufferData.sum[adc_id] += tmp;
  // write value to ring buffer
  s.adcBufferData.values[adc_id][s.adcBufferWriteIndex] = (uint32_t) value;
}

/******************************************************************************/
/**	@brief      Advance write buffer index to next position
******************************************************************************/
static inline void IPC_AdcBufferWriteNext(void)
{
  s.adcBufferWriteIndex = (s.adcBufferWriteIndex + 1) & IPC_ADC_BUFFER_MASK;
#if LPC_DGB_ADC_STRESS_TEST
  adc_val = (adc_val + 1u) & 1023u;
#endif
}

/******************************************************************************/
/**	@brief      Update read buffer index to current position
******************************************************************************/
static inline void IPC_AdcUpdateReadIndex(void)
{
  s.adcBufferReadIndex = s.adcBufferWriteIndex;
}
