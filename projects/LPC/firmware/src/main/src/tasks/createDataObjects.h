//
//  all data objects and their relationships are instantiated here
//

#pragma once

#include <stdint.h>
#include <tasks/allIOpinsTask.h>
#include "tasks/keybedScannerTask.h"
#include "tasks/ledHeartBeatM4Task.h"
#include "tasks/sensorDataWriterTask.h"
#include "tasks/usbTask.h"
#include "usb/driver/nl_usb_core_circular_buffers.h"

namespace Task
{

  //
  //  Helpers to setup the USB send/receive buffers
  //
  typedef Usb::UsbMidiSysexWriter<USB_CIRCULAR_4k>  tUsbMidiSysexWriter_4k;
  typedef Usb::UsbMidiSysexWriter<USB_CIRCULAR_8k>  tUsbMidiSysexWriter_8k;
  typedef Usb::UsbMidiSysexWriter<USB_CIRCULAR_16k> tUsbMidiSysexWriter_16k;

#define mSensorAndKeyEventBufferDeclaration(bufferSize, bufferNumber)       \
  /* bufferSize   can be "4k", "8k" or "16k" */                             \
  /* bufferNumber can be "0" or "1" (only "0" for 16k) */                   \
  typedef tUsbMidiSysexWriter_##bufferSize tUsbMidiSysexWriter_BufferSized; \
  static constexpr uint32_t *const         sensorAndKeyEventBuffer = USB_circular.buffer_##bufferSize##_##bufferNumber

  // 16k sensor buffer is good for ~300 packets, equivalent to ~0.15seconds (for a 2kHz send rate)
  mSensorAndKeyEventBufferDeclaration(16k, 0);

  //
  //  Scheduler with Dispatch and Run functions
  //
  class TaskScheduler
  {
   private:
    AllIoPins                                         m_allTimedIoPinsTask;
    LedHeartBeatM4                                    m_ledHeartBeatM4Task;
    tUsbMidiSysexWriter_BufferSized                   m_usbSensorAndKeyEventMidiSysexWriter;
    UsbProcess<tUsbMidiSysexWriter_BufferSized>       m_usbProcessTask;
    KeybedScanner<tUsbMidiSysexWriter_BufferSized>    m_keybedScannerTask;
    SensorDataWriter<tUsbMidiSysexWriter_BufferSized> m_sensorDataWriterTask;

    static inline constexpr uint32_t usToTicks(uint32_t const us)
    {
      return us / 125u;
    };

    static inline constexpr uint32_t msToTicks(uint32_t const ms)
    {
      return 1000u * ms / 125u;
    };

   public:
    constexpr TaskScheduler(void)
        // task processes pins that have timed functions
        : m_allTimedIoPinsTask(1, msToTicks(100))

        // task for heartbeat LED of M4 core
        , m_ledHeartBeatM4Task(2, msToTicks(500),
                               m_allTimedIoPinsTask.m_LED_m4HeartBeat)

        // shared MidiSysexWriter for Keybed Scanner and Sensor Scanner
        , m_usbSensorAndKeyEventMidiSysexWriter(sensorAndKeyEventBuffer, m_allTimedIoPinsTask.m_LED_usbStalling)

        // high prio task that handles the high level USB I/O
        , m_usbProcessTask(m_usbSensorAndKeyEventMidiSysexWriter)

        // high prio task for Keybed Scanner, shares a common MidiSysexWriter with Sensor Scanner
        , m_keybedScannerTask(m_allTimedIoPinsTask.m_LED_keybedEvent, m_allTimedIoPinsTask.m_LED_error,
                              m_usbSensorAndKeyEventMidiSysexWriter)

        // task for Sensor Scanner, shares a common MidiSysexWriter with Keybed Scanner
        , m_sensorDataWriterTask(3, usToTicks(500),
                                 m_allTimedIoPinsTask.m_LED_adcOverrun, m_allTimedIoPinsTask.m_LED_error,
                                 m_usbSensorAndKeyEventMidiSysexWriter) {};

    inline void dispatch(void)
    {
      m_keybedScannerTask.dispatch();
      m_sensorDataWriterTask.dispatch();
      m_usbProcessTask.dispatch();
      m_ledHeartBeatM4Task.dispatch();
      m_allTimedIoPinsTask.dispatch();
    };

    inline void run(void)
    {
      m_keybedScannerTask.run();
      m_sensorDataWriterTask.run();
      m_usbProcessTask.run();
      m_ledHeartBeatM4Task.run();
      m_allTimedIoPinsTask.run();
    };
  };

}  // namespace
