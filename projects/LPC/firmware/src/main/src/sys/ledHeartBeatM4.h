#pragma once

#include "io/pins.h"
#include "sys/mtask.h"

namespace Task
{

  class LedHeartBeatM4 : public Task::Task
  {
   public:
    using Task::Task;
    inline void body(void)
    {
      LED_M4HB = ~LED_M4HB;
    }
  };

}
