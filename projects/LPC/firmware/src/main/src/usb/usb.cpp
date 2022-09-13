#include "usb/usb.h"

static constexpr unsigned SENSOR_AND_KEY_EVENT_BUFFER_SIZE = USB_CIRCULAR_16k;
static constexpr unsigned BRIDGE_OUTPUT_BUFFER_SIZE        = USB_CIRCULAR_8k;
static constexpr unsigned BRIDGE_INPUT_BUFFER_SIZE         = USB_CIRCULAR_8k;

// 16k sensor buffer is good for ~15 packets, equivalent to ~0.15seconds (for a 2kHz send rate)
__attribute__((section(".data.$RamAHB32"))) __attribute__((aligned(SENSOR_AND_KEY_EVENT_BUFFER_SIZE))) static uint32_t sensorAndKeyEventBuffer[SENSOR_AND_KEY_EVENT_BUFFER_SIZE / sizeof(uint32_t)];

__attribute__((section(".data.$RamAHB32"))) __attribute__((aligned(BRIDGE_OUTPUT_BUFFER_SIZE))) static uint32_t bridgeOutputBuffer[BRIDGE_OUTPUT_BUFFER_SIZE / sizeof(uint32_t)];

__attribute__((section(".data.$RamAHB32"))) __attribute__((aligned(BRIDGE_INPUT_BUFFER_SIZE))) static uint32_t bridgeInputBuffer[BRIDGE_INPUT_BUFFER_SIZE / sizeof(uint32_t)];

Usb::UsbMidiWriter sensorAndKeyEventWriter(sensorAndKeyEventBuffer, sizeof sensorAndKeyEventBuffer);
