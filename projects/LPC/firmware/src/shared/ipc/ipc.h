#pragma once
#include <stdint.h>

#define LPC_DGB_ADC_STRESS_TEST (0)

#if LPC_DGB_ADC_STRESS_TEST
// use a free-running 0..4095 counter rather than real ADC values
// counter is incremented when adcBufferWriteIndex is advanced
#warning "ADC stress test is on, no real ADC values will be used"
static uint16_t adc_val;
#endif

//
//  -------- ADC --------
//
#define IPC_ADC_NUMBER_OF_CHANNELS (32u)

// ADC ring buffers
// Must be 2^N in size and <= 16. This also determines the averaging.
// Size should NOT be larger than the number of aquisitions between M4 read-out operations
#define IPC_ADC_BUFFER_SIZE (4u)
#define IPC_ADC_BUFFER_MASK (IPC_ADC_BUFFER_SIZE - 1)
#define IPC_ADC_DEFAULT     (512u)

typedef struct
{
  uint32_t values[IPC_ADC_NUMBER_OF_CHANNELS][IPC_ADC_BUFFER_SIZE];
  int32_t  sumTmp[IPC_ADC_NUMBER_OF_CHANNELS];
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
  volatile uint32_t ticker;
  volatile uint32_t adcCycleFinished;
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
} SharedData_T;

extern SharedData_T s;

/******************************************************************************/
/**	@brief  setup and clear data in shared memory
*******************************************************************************/
inline static void IPC_Init(void)
{
  s.ticker            = 0;
  s.adcCycleFinished  = 0;
  s.keyBufferWritePos = 0;
  s.keyBufferReadPos  = 0;
  s.M0_KbsIrqOvers    = 0;

  for (unsigned i = 0; i < IPC_ADC_NUMBER_OF_CHANNELS; i++)
  {
    for (unsigned k = 0; k < IPC_ADC_BUFFER_SIZE; k++)
      s.adcBufferData.values[i][k] = IPC_ADC_DEFAULT;
    s.adcBufferData.sum[i] = s.adcBufferData.sumTmp[i] = IPC_ADC_DEFAULT * IPC_ADC_BUFFER_SIZE;
  }
  s.adcBufferReadIndex  = 0;
  s.adcBufferWriteIndex = 0;
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
    @param[in]  pKeyEvent: pointer to an array of key events
                that will be processed by the voice allocation
    @return     Number of new key events (0: nothing to do)
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
/**	@brief      Read ADC channel value
*   @param[in]	IPC id of the adc channel 0...15
*   @return     adc channel value
******************************************************************************/
static inline uint32_t IPC_ReadAdcBuffer(unsigned const adc_id)
{
  // M0 may advance adcBufferReadIndex while we are reading values for a number
  // of ADCs during an M4 time-slice. This means values of different ADCs may
  // not all be from the same conversion cycle of M0. Since the conversion cycle
  // runs 4 times per M4 time-slice the error is neglegible, though.
  return s.adcBufferData.values[adc_id][s.adcBufferReadIndex];
}

/******************************************************************************/
/**	@brief      Read ADC channel value as average of whole ring buffer contents
*   @param[in]	IPC id of the adc channel 0...15
*   @return     adc channel value
******************************************************************************/
static inline uint32_t IPC_ReadAdcBufferAveraged(unsigned const adc_id)
{
  // Again, values in the buffers for different ADCs may not be in sync with
  // M4's time-slice, but because of the averaging the error is even smaller.
  // More importantly, M0 may be just in the middle of its non-atomic
  // read-modify-write operation in IPC_WriteAdcBuffer, but again the error
  // is very small because only a delta is added/subtracted.
  return ((uint32_t) s.adcBufferData.sum[adc_id] + IPC_ADC_BUFFER_SIZE / 2u) / IPC_ADC_BUFFER_SIZE;
}

/******************************************************************************/
/**	@brief      Read ADC channel value as sum of whole ring buffer contents
*   @param[in]	IPC id of the adc channel 0...31
*   @return     adc channel value
******************************************************************************/
static inline uint32_t IPC_ReadAdcBufferSum(unsigned const adc_id)
{
  // see notes for IPC_ReadAdcBufferAveraged
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
  s.adcBufferData.sumTmp[adc_id] += -((int32_t)(s.adcBufferData.values[adc_id][s.adcBufferWriteIndex])) + (int32_t) value;
  // write value to ring buffer
  s.adcBufferData.values[adc_id][s.adcBufferWriteIndex] = (uint32_t) value;
}

static inline void IPC_CopyAdcData(void)
{
  for (unsigned i = 0; i < IPC_ADC_NUMBER_OF_CHANNELS; i++)
    s.adcBufferData.sum[i] = s.adcBufferData.sumTmp[i];
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