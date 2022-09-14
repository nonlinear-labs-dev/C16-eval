#pragma once
#include <stdint.h>
#include <tasks/allIOpinsTask.h>
#include "tasks/keybedScannerTask.h"
#include "tasks/ledHeartBeatM4Task.h"
#include "tasks/sensorDataWriterTask.h"
#include "tasks/usbTask.h"

#include "usb/driver/nl_usb_core.h"

static constexpr unsigned SENSOR_AND_KEY_EVENT_BUFFER_SIZE = USB_CIRCULAR_16k;
// 16k sensor buffer is good for ~15 packets, equivalent to ~0.15seconds (for a 2kHz send rate)
__attribute__((section(".data.$RamAHB32"))) __attribute__((aligned(SENSOR_AND_KEY_EVENT_BUFFER_SIZE))) static uint32_t sensorAndKeyEventBuffer[SENSOR_AND_KEY_EVENT_BUFFER_SIZE / sizeof(uint32_t)];
#warning ^ OBACHT, groesse muss zu untigem typedef UsbMidiWriter_BufferSized passen!
//
//  Scheduler with Dispatch and Run functions
//
namespace Task
{
  typedef Usb::UsbMidiWriter<USB_CIRCULAR_4k>  UsbMidiWriter_4k;
  typedef Usb::UsbMidiWriter<USB_CIRCULAR_8k>  UsbMidiWriter_8k;
  typedef Usb::UsbMidiWriter<USB_CIRCULAR_16k> UsbMidiWriter_16k;

  typedef UsbMidiWriter_16k UsbMidiWriter_BufferSized;

  class Scheduler
  {
   private:
    AllIoPins                                   m_allTimedIoPinsTask;
    LedHeartBeatM4                              m_ledHeartBeatM4Task;
    UsbMidiWriter_BufferSized                   m_usbSensorAndKeyEventMidiWriter;
    UsbProcess<UsbMidiWriter_BufferSized>       m_usbProcessTask;
    KeybedScanner<UsbMidiWriter_BufferSized>    m_keybedScannerTask;
    SensorDataWriter<UsbMidiWriter_BufferSized> m_sensorDataWriterTask;

    inline uint32_t usToTicks(uint32_t const us)
    {
      return us / 125u;
    };

    inline uint32_t msToTicks(uint32_t const ms)
    {
      return 1000u * ms / 125u;
    };

   public:
    Scheduler(void)
        : m_allTimedIoPinsTask(1, msToTicks(100))
        , m_ledHeartBeatM4Task(2, msToTicks(500), m_allTimedIoPinsTask.getLED_m4HeartBeat())
        , m_usbSensorAndKeyEventMidiWriter(sensorAndKeyEventBuffer, m_allTimedIoPinsTask.getLED_usbStalling())
        , m_usbProcessTask(m_usbSensorAndKeyEventMidiWriter)
        , m_keybedScannerTask(m_allTimedIoPinsTask.getLED_keybedEvent(), m_allTimedIoPinsTask.getLED_error(),
                              m_usbSensorAndKeyEventMidiWriter)
        , m_sensorDataWriterTask(3, usToTicks(500),
                                 m_allTimedIoPinsTask.getLED_adcOverrun(), m_allTimedIoPinsTask.getLED_error(),
                                 m_usbSensorAndKeyEventMidiWriter) {};

    inline void dispatch(void)
    {
      m_ledHeartBeatM4Task.dispatch();
      m_allTimedIoPinsTask.dispatch();
      m_keybedScannerTask.dispatch();
      m_sensorDataWriterTask.dispatch();
      m_usbProcessTask.dispatch();
    };

    inline void run(void)
    {
      m_ledHeartBeatM4Task.run();
      m_allTimedIoPinsTask.run();
      m_keybedScannerTask.run();
      m_sensorDataWriterTask.run();
      m_usbProcessTask.run();
    };
  };

}  // namespace
