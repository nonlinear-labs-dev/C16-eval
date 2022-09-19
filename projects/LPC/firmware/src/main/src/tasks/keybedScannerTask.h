#pragma once

#include "drv/IoPin.h"
#include "ipc/ipc.h"
#include "tasks/mtask.h"
#include "usb/usb.h"

namespace Task
{
  static constexpr unsigned KEYBED_DATA_CABLE_NUMBER = 1u;

  template <typename tUsbMidiWriter>
  class KeybedScanner : public Task::Task
  {
    using Task::Task;

   private:
    unsigned        tan;
    IOpins::IOpin&  m_keybedEventLED;
    IOpins::IOpin&  m_dataLossLED;
    tUsbMidiWriter& m_keyEventWriter;

   public:
    constexpr KeybedScanner(IOpins::IOpin& keybedEventLED, IOpins::IOpin& dataLossLED, tUsbMidiWriter& keyEventWriter)
        : Task()
        , m_keybedEventLED(keybedEventLED)
        , m_dataLossLED(dataLossLED)
        , m_keyEventWriter(keyEventWriter) {};

    // no dispatcher and body needed
    inline void dispatch(void) {};
    inline void body(void) {};

    // scanner is run unconditionally
    inline void run(void)
    {
      uint32_t event = 0;
      while ((event = IPC_M4_KeyBuffer_ReadBuffer()))  // reads the latest key up/down events from M0 ring buffer
      {
        // tan
        tan = (tan + 1u) & 0b11111111111111;

        if (m_keyEventWriter.claimBufferElements(3))  // 3 4-byte frames available ?
        {
          m_keyEventWriter.write(KEYBED_DATA_CABLE_NUMBER, 0xF0, Usb::getSysexHiByte(tan), Usb::getSysexLoByte(tan));                              // start, tanH/tanL
          m_keyEventWriter.write(KEYBED_DATA_CABLE_NUMBER, Usb::getSysexLoByte(event), Usb::getSysexHi4Byte(event), Usb::getSysexHi3Byte(event));  // keynum+make, timeHH/HL
          m_keyEventWriter.writeLast(KEYBED_DATA_CABLE_NUMBER, Usb::getSysexHi2Byte(event), Usb::getSysexHiByte(event), 0xF7);                     // timeLH/LL, end
        }
        else  // Data Loss !!!
          m_dataLossLED.timedOn(1);
      }
      if (event)
        m_keybedEventLED.timedOn(2);
    };
  };

}  // namespace
