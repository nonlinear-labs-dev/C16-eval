#pragma once

#include "drv/allIoPins.h"
#include "tasks/mtask.h"

namespace Task
{

  class LedHeartBeatM4 : public Task::Task
  {
    using Task::Task;

   public:
    inline void body(void)
    {
      LED_M4HeartBeat.toggle();
    }
  };

}
