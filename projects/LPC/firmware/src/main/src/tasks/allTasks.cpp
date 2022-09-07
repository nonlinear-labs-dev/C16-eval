#include <stdint.h>
#include "tasks/allTasks.h"
#include "drv/allIoPins.h"

static inline uint32_t usToTicks(uint32_t const us)
{
  return us / 125u;
}

static inline uint32_t msToTicks(uint32_t const ms)
{
  return 1000u * ms / 125u;
}

// clang-format off
//                                     delay,         period
Task::KeybedScanner    keybedScanner   (   0,              0 );
Task::AllTimedIoPins      allTimedIoPins     (   1, msToTicks( 100));
Task::LedHeartBeatM4   ledHeartBeatM4  (   2, msToTicks( 500));
Task::SensorDataWriter sensorDataWriter(   3, usToTicks( 500));
// clang-format on
