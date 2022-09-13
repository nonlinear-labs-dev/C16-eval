#include "usb/usb.h"
#include "ipc/ipc.h"

 #define __MEMSPEC__ __attribute__((section(".data.$RamAHB32")))
__MEMSPEC__ uint32_t sensorAndKeyeventUsbBuffer[1024];  // must be 4k size and start at a 4k boundary !!!


Usb::UsbMidiSysexWriter sensorAndKeyEventWriter(sensorAndKeyeventUsbBuffer);
