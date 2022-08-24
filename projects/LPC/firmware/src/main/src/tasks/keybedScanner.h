#pragma once

#include "ipc/ipc.h"
#include "tasks/mtask.h"

namespace Task
{

  class KeybedScanner : public Task::Task
  {
    using Task::Task;

   private:
    uint32_t m_keyEvent[32];  // array for new events read from the ring buffer for keybed events

   public:
    KeybedScanner(uint32_t delay, uint32_t period)
        : Task(delay, period) {};

    inline virtual void body(void)
    {
      unsigned numKeyEvents = IPC_M4_KeyBuffer_ReadBuffer(m_keyEvent, 32);  // reads the latest key up/down events from M0 ring buffer
      if (numKeyEvents)
        ;  // TODO
    };
  };

}
