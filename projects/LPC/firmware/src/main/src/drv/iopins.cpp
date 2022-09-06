#include "drv/iopins.h"
#include "io/pins.h"

IOpins::IOpin LED_M4HeartBeat(&LED_M4HB);
IOpins::IOpin LED_Warning(&LED_WARNING);
IOpins::IOpin LED_Error(&LED_ERROR);
IOpins::IOpin LED_KeybedEvent(&LED_E);
IOpins::IOpin LED_USBstalling(&LED_I);
