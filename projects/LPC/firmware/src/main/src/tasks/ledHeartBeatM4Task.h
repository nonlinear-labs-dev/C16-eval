#pragma once

#include "drv/IoPin.h"
#include "tasks/mtask.h"

namespace Task
{

  class LedHeartBeatM4 : public Task::Task
  {
   private:
    IOpins::IOpin& m_M4HeartBeatLED;

   public:
    constexpr LedHeartBeatM4(uint32_t const delay, uint32_t const period, IOpins::IOpin& M4HeartBeatLED)
        : Task(delay, period)
        , m_M4HeartBeatLED(M4HeartBeatLED) {};

    inline void body(void)
    {
      m_M4HeartBeatLED.toggle();
    }
  };

}
