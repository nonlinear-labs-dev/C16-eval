#pragma once

#include "drv/allIoPins.h"
#include "tasks/mtask.h"

namespace Task
{

  class AllTimedIoPins : public Task::Task
  {
    using Task::Task;

   public:
    inline void body(void)
    {
      LED_Warning.process();
      LED_Error.process();
      LED_KeybedEvent.process();
      LED_USBstalling.process();
    };
  };

}
