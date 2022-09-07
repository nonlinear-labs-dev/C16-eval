#pragma once

//
// Task Objects (items further down in the list may use previous objects)
// -- actual objects are instantiated in allTasks.cpp
//
#include "tasks/allTimedIOpins.h"
extern Task::AllTimedIoPins allTimedIoPins;

#include "tasks/ledHeartBeatM4.h"
extern Task::LedHeartBeatM4 ledHeartBeatM4;

#include "tasks/keybedScanner.h"
extern Task::KeybedScanner keybedScanner;

#include "tasks/sensorDataWriter.h"
extern Task::SensorDataWriter sensorDataWriter;

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
  };

  inline void run(void)
  {
    ledHeartBeatM4.run();
    allTimedIoPins.run();
    keybedScanner.run();
    sensorDataWriter.run();
  }
}
