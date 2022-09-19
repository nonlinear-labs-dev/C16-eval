#include <stdint.h>

#include "usb/driver/nl_usb_core_circular_buffers.h"

// must use the 40kByte ram segment RamLoc40 (as per "ld/main_memory.ld")
// placing it in ".noninit" section frees startup-code from zeroing this data
__attribute__((section(".noinit.$RamLoc40"))) __attribute__((aligned(16384u)))
USB_circularBuffers_t USB_circular;
