#pragma once

#include "ipc/ipc.h"
#include "tasks/mtask.h"

namespace Task
{

  class Adc : public Task::Task
  {
    using Task::Task;

   private:
    int32_t m_sum[IPC_ADC_NUMBER_OF_CHANNELS];

   public:
    inline void body(void)
    {
      for (unsigned i = 0; i < IPC_ADC_NUMBER_OF_CHANNELS; i++)
        m_sum[i] = IPC_ReadAdcBufferSum(i);
    }
  };

}
