#pragma once

#include "drv/allIoPins.h"
#include "ipc/ipc.h"
#include "tasks/mtask.h"
#include "usb/usb.h"

namespace Task
{
  static constexpr unsigned KEYBED_DATA_CABLE_NUMBER = 1u;

  class KeybedScanner : public Task::Task
  {
    using Task::Task;

   private:
    unsigned tan;

   public:
    // no dispatcher and body needed
    inline void dispatch(void)
    {
    }

    inline void body(void)
    {
    }

    // scanner is run unconditionally
    inline void run(void)
    {
      uint32_t event;
      if ((event = IPC_M4_KeyBuffer_ReadBuffer()))  // reads the latest key up/down events from M0 ring buffer
      {
        LED_KeybedEvent.timedOn(2);

        // tan
        tan = (tan + 1u) & 0b11111111111111;

        if (sensorAndKeyEventWriter.claimBuffer(3))  // 3 4-byte frames available ?
        {
          sensorAndKeyEventWriter.write(KEYBED_DATA_CABLE_NUMBER, 0xF0, getSysexHiByte(tan), getSysexLoByte(tan));                         // start, tanH/tanL
          sensorAndKeyEventWriter.write(KEYBED_DATA_CABLE_NUMBER, getSysexLoByte(event), getSysexHi4Byte(event), getSysexHi3Byte(event));  // keynum+make, timeHH/HL
          sensorAndKeyEventWriter.writeLast(KEYBED_DATA_CABLE_NUMBER, getSysexHi2Byte(event), getSysexHiByte(event), 0xF7);                // timeLH/LL, end
        }
        else  // Data Loss !!!
          LED_Error.timedOn(1);
      }
    };
  };

}
