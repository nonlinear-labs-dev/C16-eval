//
//  all data objects and their relationships are instantiated here
//

#pragma once

#include <stdint.h>
#include "tasks/lraTask.h"
#include "tasks/uartTask.h"
#include "tasks/allIOpinsTask.h"
#include "tasks/statemonitor.h"
#include "tasks/keybedScannerTask.h"
#include "tasks/ledHeartBeatM4Task.h"
#include "tasks/sensorDataWriterTask.h"
#include "tasks/encoderTask.h"
#include "tasks/usbTask.h"
#include "drv/LRA.h"
#include "sys/ticker.h"

#include "usb/usbWriter.h"
#include "usb/bridgeDataUsbWriter.h"

namespace Task
{
  //
  //  Scheduler with Dispatch and Run functions
  //
  class TaskScheduler
  {
   public:
    TaskScheduler(void)
    {
      ticker = 0;
    }

   private:
    // task processes pins that have timed functions
    AllIoPins m_allTimedIoPinsTask { 1, msToTicks(100) };

    // state monitor
    StateMonitor::StateMonitor m_stateMonitor { m_allTimedIoPinsTask };

    // task for heartbeat LED of M4 core
    LedHeartBeatM4 m_ledHeartBeatM4Task { 2, msToTicks(500),
                                          m_allTimedIoPinsTask.m_LED_m4HeartBeat };

    // shared MidiSysexWriter for Keybed Scanner and Sensor Scanner to Host (USB0 HS)
    Usb::UsbMidiSysexWriter m_usbSensorAndKeyEventMidiSysexWriter { Usb::USBPorts::USB0, m_stateMonitor };

    // USB Writer for Data from Host (USB0 HS) to Bridge (USB1 FS)
    Usb::UsbBridgeWriter m_hostToBridge { Usb::USBPorts::USB1, m_stateMonitor };

    // USB Writer for Data from Bridge (USB1 FS) To Host USB0 HS)
    Usb::UsbBridgeWriter m_bridgeToHost { Usb::USBPorts::USB0, m_stateMonitor };

    // high prio task that handles the high level USB I/O
    UsbProcess m_usbProcessTask { m_usbSensorAndKeyEventMidiSysexWriter,
                                  m_bridgeToHost,
                                  m_hostToBridge };

    // high prio task for Keybed Scanner, shares a common MidiSysexWriter with Sensor Scanner
    KeybedScanner m_keybedScannerTask { m_usbSensorAndKeyEventMidiSysexWriter,
                                        m_stateMonitor };

    // Interrupt task for Rotary Encoder, runs at 4x speed (31us)
    Encoder m_encoder;

    // task for Sensor Scanner, shares a common MidiSysexWriter with Keybed Scanner
    SensorDataWriter m_sensorDataWriterTask { 3, usToTicks(500),
                                              m_usbSensorAndKeyEventMidiSysexWriter,
                                              m_stateMonitor,
                                              m_encoder };

    // task for LRA handling
    LRAHandler m_lraTask { 4, usToTicks(LraHardware::resonancePeriodInMicroseconds),
                           m_allTimedIoPinsTask.m_LED_lraActivity };

    // high prio task for uart processing
    Uart m_uartTask { m_allTimedIoPinsTask.m_LED_uartActivity, m_allTimedIoPinsTask.m_LED_uartError, m_lraTask };

   public:
    inline void dispatch(void)
    {
      ticker++;
      m_keybedScannerTask.dispatch();
      m_usbProcessTask.dispatch();

      m_sensorDataWriterTask.dispatch();
      m_uartTask.dispatch();
      m_ledHeartBeatM4Task.dispatch();
      m_lraTask.dispatch();

      m_allTimedIoPinsTask.dispatch();
    };

    inline void run(void)
    {
      m_keybedScannerTask.run();
      m_usbProcessTask.run();

      m_sensorDataWriterTask.run();
      m_uartTask.run();
      m_ledHeartBeatM4Task.run();
      m_lraTask.run();

      m_allTimedIoPinsTask.run();
    };

    // this function is run at 4x speed from within the timer interrupt
    inline void runTIRQ4x(void)
    {
      m_encoder.run();
    }
  };

}  // namespace
