#pragma once

#include "ipc/ipc.h"
#include "tasks/mtask.h"

namespace Task
{

  class Adc : public Task::Task
  {
    using Task::Task;

   private:
    union
    {
      struct
      {
        struct
        {
          int32_t w0;
          int32_t w1;
        } erp[8];
        int32_t ehc[8];
        int32_t aftertouch;
        int32_t bender;
        int32_t ribbon[2];
        int32_t light[2];
        int32_t mon19V;
        int32_t mon5V;

      } data;
      int32_t array[32];
    } ch;

   public:
    // no dispatcher and body needed
    inline void dispatch(void)
    {
    }

    inline void body(void)
    {
    }

    // we check for the signal from M0 core here and run the job directly
    inline void run(void)
    {
      if (s.adcCycleFinished)
      {
        s.adcCycleFinished = 0;
        DBG_ADC_CYCLE      = ~DBG_ADC_CYCLE;
        for (unsigned i = 0; i < IPC_ADC_NUMBER_OF_CHANNELS; i++)
          ch.array[i] = IPC_ReadAdcBufferSum(i);
      }
    }
  };

}
