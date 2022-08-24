#pragma once
#include <stdint.h>

#define IPC_KEYBUFFER_SIZE       (64u)  // number of key events that can be processed in 125us
#define IPC_KEYBUFFER_MASK       (IPC_KEYBUFFER_SIZE - 1u)
#define IPC_KEYBUFFER_KEYMASK    (0x3F)
#define IPC_KEYBUFFER_NOTEON     (0x40)
#define IPC_KEYBUFFER_TIME_SHIFT (7u)

typedef struct
{
  volatile uint32_t ticker;
  uint32_t          keyBufferData[IPC_KEYBUFFER_SIZE];
#ifdef CORE_M4
  volatile
#endif
      uint32_t keyBufferWritePos;
  uint32_t     keyBufferReadPos;
  uint32_t     M0_KbsIrqOvers;
} SharedData_T;

extern SharedData_T s;

/******************************************************************************/
/**	@brief  setup and clear data in shared memory
*******************************************************************************/
inline static void IPC_Init(void)
{
  s.ticker            = 0;
  s.keyBufferWritePos = 0;
  s.keyBufferReadPos  = 0;
  s.M0_KbsIrqOvers    = 0;
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
  // isn't update so fast as to overrun keyBufferReadPos nothing bad will happe, though.
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
                maxNumOfEventsToRead: size of the array pointed by pKeyEvent
    @return     Number of new key events (0: nothing to do)
*******************************************************************************/
static inline unsigned IPC_M4_KeyBuffer_ReadBuffer(uint32_t* const pKeyEvent, unsigned const maxNumOfEventsToRead)
{
  // !! this is a potentially critical section !!
  // Emphase_IPC_M0_KeyBuffer_WriteKeyEvent() should not run while we are here
  // because it updates keyBufferWritePos.
  // As long as keyBufferWritePos, which is volatile from M4's view, doesn't update so fast
  // that it overruns keyBufferReadPos (which is extremely unlikely, only when Emphase_IPC_M4_KeyBuffer_ReadBuffer
  // is interrupted/stalling for a really long time), nothing bad will happen, though.
  uint8_t count = 0;
  while ((s.keyBufferReadPos != s.keyBufferWritePos) && (count < maxNumOfEventsToRead))
  {
    pKeyEvent[count]   = s.keyBufferData[s.keyBufferReadPos];
    s.keyBufferReadPos = (s.keyBufferReadPos + 1) & (IPC_KEYBUFFER_MASK);
    count++;
  }
  return count;
}

/******************************************************************************/
/** @brief      Return size of key buffer
*******************************************************************************/
static inline unsigned IPC_KeyBuffer_GetSize()
{
  return IPC_KEYBUFFER_SIZE;
}
