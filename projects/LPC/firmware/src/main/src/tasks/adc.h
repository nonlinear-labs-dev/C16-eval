#pragma once

#include "ipc/ipc.h"
#include "tasks/mtask.h"
#include "usb/nl_usb_midi.h"
#include "io/pins.h"
#include "usbmidisysex/usbMidiSysex.h"

namespace Task
{
  class Adc : public Task::Task
  {
    using Task::Task;

   private:
    unsigned          m_transactionNumber = 0u;
    UsbMidiSysex::Tan tan;
    UsbMidiSysex::Erp erp;

   public:
    inline void body(void)
    {

      LED_E               = ~LED_E;
      m_transactionNumber = tan.next(m_transactionNumber);
      tan.write(m_transactionNumber);

      erp.write(0, IPC_ReadAdcBufferSum(0), IPC_ReadAdcBufferSum(1));
      erp.write(1, IPC_ReadAdcBufferSum(2), IPC_ReadAdcBufferSum(3));
      erp.write(2, IPC_ReadAdcBufferSum(4), IPC_ReadAdcBufferSum(5));
      erp.write(3, IPC_ReadAdcBufferSum(6), IPC_ReadAdcBufferSum(7));
      erp.write(4, IPC_ReadAdcBufferSum(8), IPC_ReadAdcBufferSum(9));
      erp.write(5, IPC_ReadAdcBufferSum(10), IPC_ReadAdcBufferSum(11));

      LED_ERROR = (USB_MIDI_Send(0, (uint8_t *) (&UsbMidiSysex::sensorAndStatusData), sizeof UsbMidiSysex::sensorAndStatusData) == -1);
    }
  };

}
