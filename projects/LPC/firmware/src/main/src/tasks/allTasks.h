#pragma once
#include <stdint.h>

#include "tasks/allTimedIOpins.h"
#include "tasks/ledHeartBeatM4.h"
#include "tasks/keybedScanner.h"
#include "tasks/sensorDataWriter.h"
#include "tasks/usb.h"

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
Task::UsbWriter        usbWriter       (   0,              0 );
Task::AllTimedIoPins   allTimedIoPins  (   1, msToTicks( 100));
Task::LedHeartBeatM4   ledHeartBeatM4  (   2, msToTicks( 500));
Task::SensorDataWriter sensorDataWriter(   3, usToTicks( 500));
// clang-format on

//
//  Dispatcher/Scheduler and Run functions
//
namespace Task
{

  inline void dispatch(void)
  {
    ledHeartBeatM4.dispatch();
    allTimedIoPins.dispatch();
    keybedScanner.dispatch();
    sensorDataWriter.dispatch();
    usbWriter.dispatch();
  };

  inline void run(void)
  {
    ledHeartBeatM4.run();
    allTimedIoPins.run();
    keybedScanner.run();
    sensorDataWriter.run();
    usbWriter.run();
  }
}
