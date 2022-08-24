#include "tasks/tasks.h"

Task::KeybedScanner   keybedScanner(0u, 10u);
Task::LedErrorWarning ledErrorWarning(1u, 800u);
Task::LedHeartBeatM4  ledHeartBeatM4(2u, 4000u);
