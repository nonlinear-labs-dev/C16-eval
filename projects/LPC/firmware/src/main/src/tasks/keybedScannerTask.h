#pragma once

#include "drv/IoPin.h"
#include "ipc/ipc.h"
#include "tasks/mtask.h"
#include "tasks/statemonitor.h"
#include "usb/usbWriter_MidiSysex.h"
#include "usb/sysexFunctions.h"

namespace Task
{
  static constexpr unsigned KEYBED_DATA_CABLE_NUMBER = 1u;

  class KeybedScanner : public Task::Task
  {
    using Task::Task;

   private:
    unsigned                    m_tan { 0 };
    UsbWriter::MidiSysexWriter& m_keyEventWriter;
    StateMonitor::StateMonitor& m_stateMonitor;

   public:
    constexpr KeybedScanner(UsbWriter::MidiSysexWriter& keyEventWriter, StateMonitor::StateMonitor& stateMonitor)
        : Task()
        , m_keyEventWriter(keyEventWriter)
        , m_stateMonitor(stateMonitor) {};

    // no dispatcher and body needed
    inline void dispatch(void) {};
    inline void body(void) {};

    // scanner is run unconditionally
    inline void run(void)
    {
      uint32_t event;
      int      hadEvent = 0;
      while ((event = IPC_M4_KeyBuffer_ReadBuffer()))  // reads the latest key up/down events from M0 ring buffer
      {
        hadEvent = 1;
        // tan
        m_tan = (m_tan + 1u) & 0b11111111111111;

        if (m_keyEventWriter.claimBufferElements(3))  // 3 4-byte frames available ?
        {
          m_keyEventWriter.write(KEYBED_DATA_CABLE_NUMBER,
                                 0xF0,                           // Sysex start
                                 SysEx::getSysexHiByte(m_tan),   // tanH
                                 SysEx::getSysexLoByte(m_tan));  // tanL
          m_keyEventWriter.write(KEYBED_DATA_CABLE_NUMBER,
                                 SysEx::getSysexLoByte(event),    // keynum+make,
                                 SysEx::getSysexHi4Byte(event),   // timeHH
                                 SysEx::getSysexHi3Byte(event));  // timeHL
          m_keyEventWriter.writeLast(KEYBED_DATA_CABLE_NUMBER,
                                     SysEx::getSysexHi2Byte(event),  // timeLH
                                     SysEx::getSysexHiByte(event),   // timeLL
                                     0xF7);                          // Sysex end
        }
        else  // Data Loss !!!
          m_stateMonitor.event(StateMonitor::ERROR_KEYBED_DATA_LOSS);
      }
      if (hadEvent)
        m_stateMonitor.event(StateMonitor::INFO_KEYBED_EVENT);
    };
  };

}  // namespace
