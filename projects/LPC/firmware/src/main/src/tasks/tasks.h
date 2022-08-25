#pragma once

//
// Task Objects (items further down in the list may use previous objects)
// -- actual objects are instantiated in tasks.cpp
//
#include "tasks/ledErrorWarning.h"
extern Task::LedErrorWarning ledErrorWarning;

#include "tasks/ledHeartBeatM4.h"
extern Task::LedHeartBeatM4 ledHeartBeatM4;

#include "tasks/keybedScanner.h"
extern Task::KeybedScanner keybedScanner;

//
//  Dispatcher/Scheduler and Run functions
//
namespace Task
{

  inline void dispatch(void)
  {
    ledHeartBeatM4.dispatch();
    ledErrorWarning.dispatch();
    keybedScanner.dispatch();
  };

  inline void run(void)
  {
    (void) ledHeartBeatM4.run();
    (void) ledErrorWarning.run();
    (void) keybedScanner.run();
  }
}
