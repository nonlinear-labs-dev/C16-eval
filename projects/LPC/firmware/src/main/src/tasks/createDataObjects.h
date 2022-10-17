//
//  all data objects and their relationships are instantiated here
//

#pragma once

#include <stdint.h>
#include "tasks/lraTask.h"
#include "tasks/uartEchoTestTask.h"
#include "tasks/allIOpinsTask.h"
#include "tasks/statemonitor.h"
#include "tasks/keybedScannerTask.h"
#include "tasks/ledHeartBeatM4Task.h"
#include "tasks/sensorDataWriterTask.h"
#include "tasks/usbTask.h"
#include "tasks/lraTask.h"
#include "usb/driver/nl_usb_core_circular_buffers.h"

namespace Task
{
  //
  //  Scheduler with Dispatch and Run functions
  //
  class TaskScheduler
  {
   private:
    // task processes pins that have timed functions
    AllIoPins m_allTimedIoPinsTask { 1, msToTicks(100) };

    // state monitor
    StateMonitor::StateMonitor m_stateMonitor { m_allTimedIoPinsTask };

    // task for heartbeat LED of M4 core
    LedHeartBeatM4 m_ledHeartBeatM4Task { 2, msToTicks(500),
                                          m_allTimedIoPinsTask.m_LED_m4HeartBeat };

    // shared MidiSysexWriter for Keybed Scanner and Sensor Scanner
    Usb::UsbMidiSysexWriter m_usbSensorAndKeyEventMidiSysexWriter { m_stateMonitor };

    // high prio task that handles the high level USB I/O
    UsbProcess m_usbProcessTask { m_usbSensorAndKeyEventMidiSysexWriter };

    // high prio task for Keybed Scanner, shares a common MidiSysexWriter with Sensor Scanner
    KeybedScanner m_keybedScannerTask { m_usbSensorAndKeyEventMidiSysexWriter,
                                        m_stateMonitor };

    // task for Sensor Scanner, shares a common MidiSysexWriter with Keybed Scanner
    SensorDataWriter m_sensorDataWriterTask { 3, usToTicks(500),
                                              m_usbSensorAndKeyEventMidiSysexWriter,
                                              m_stateMonitor };

    // task for uart processing
    UartEchoTest m_uartEchoTestTask { m_allTimedIoPinsTask.m_LED_uartActivity, m_allTimedIoPinsTask.m_LED_uartError };

    // task for LRA handling
    LRAHandler m_lraTask { 4, usToTicks(LRA::resonancePeriodInMicroseconds),
                           m_allTimedIoPinsTask.m_LED_lraActivity };

    static inline constexpr uint32_t usToTicks(uint32_t const us)
    {
      return us / 125u;
    };

    static inline constexpr uint32_t msToTicks(uint32_t const ms)
    {
      return 1000u * ms / 125u;
    };

   public:
    inline void dispatch(void)
    {
      m_keybedScannerTask.dispatch();
      m_usbProcessTask.dispatch();

      m_sensorDataWriterTask.dispatch();
      m_uartEchoTestTask.dispatch();
      m_ledHeartBeatM4Task.dispatch();
      m_lraTask.dispatch();

      m_allTimedIoPinsTask.dispatch();
    };

    inline void run(void)
    {
      m_keybedScannerTask.run();
      m_usbProcessTask.run();

      m_sensorDataWriterTask.run();
      m_uartEchoTestTask.run();
      m_ledHeartBeatM4Task.run();
      m_lraTask.run();

      m_allTimedIoPinsTask.run();
    };
  };

}  // namespace
