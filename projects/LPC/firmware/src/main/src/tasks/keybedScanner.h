#pragma once

#include "ipc/ipc.h"
#include "tasks/mtask.h"

namespace Task
{

  class KeybedScanner : public Task::Task
  {
    using Task::Task;

   public:
    KeybedScanner(uint32_t delay, uint32_t period)
        : Task(delay, period) {};

    // no dispatcher needed
    inline void dispatch(void)
    {
    }

    // scanner is run unconditionally
    inline void run(void)
    {
      uint32_t event;
      while ((event = IPC_M4_KeyBuffer_ReadBuffer()))  // reads the latest key up/down events from M0 ring buffer
        ledErrorWarning.Warning_TimedOn(1);
    };
  };

}
