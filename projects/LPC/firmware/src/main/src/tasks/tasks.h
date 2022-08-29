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

#include "tasks/adc.h"
extern Task::Adc adc;

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
    adc.dispatch();
  };

  inline void run(void)
  {
    ledHeartBeatM4.run();
    ledErrorWarning.run();
    keybedScanner.run();
    adc.run();
  }
}
