#pragma once

#include "ipc/ipc.h"
#include "tasks/mtask.h"
#include "drv/iopins.h"

namespace Task
{

  class KeybedScanner : public Task::Task
  {
    using Task::Task;

   public:
    // no dispatcher and body needed
    inline void dispatch(void)
    {
    }

    inline void body(void)
    {
    }

    // scanner is run unconditionally
    inline void run(void)
    {
      uint32_t event;
      while ((event = IPC_M4_KeyBuffer_ReadBuffer()))  // reads the latest key up/down events from M0 ring buffer
        LED_KeybedEvent.timedOn(3);
    };
  };

}
