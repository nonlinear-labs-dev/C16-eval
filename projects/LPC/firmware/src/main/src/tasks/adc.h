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

    UsbMidiSysex::Tan     tan;
    UsbMidiSysex::Erp     erp;
    UsbMidiSysex::Status  status;
    UsbMidiSysex::Ehc     ehc;
    UsbMidiSysex::Ihc     ihc;
    UsbMidiSysex::Ali     ali;
    UsbMidiSysex::Psu     psu;
    UsbMidiSysex::RotEnc  rotenc;
    UsbMidiSysex::Buttons buttons;
    UsbMidiSysex::Spare   spare;

   public:
    inline void body(void)
    {

      LED_E = ~LED_E;
      tan.next();
      tan.write();

      erp.write(0, IPC_ReadAdcBufferSum(0), IPC_ReadAdcBufferSum(1));
      erp.write(1, IPC_ReadAdcBufferSum(2), IPC_ReadAdcBufferSum(3));
      erp.write(2, IPC_ReadAdcBufferSum(4), IPC_ReadAdcBufferSum(5));
      erp.write(3, IPC_ReadAdcBufferSum(6), IPC_ReadAdcBufferSum(7));
      erp.write(4, IPC_ReadAdcBufferSum(8), IPC_ReadAdcBufferSum(9));
      erp.write(5, IPC_ReadAdcBufferSum(10), IPC_ReadAdcBufferSum(11));
      erp.write(6, IPC_ReadAdcBufferSum(12), IPC_ReadAdcBufferSum(13));
      erp.write(7, IPC_ReadAdcBufferSum(14), IPC_ReadAdcBufferSum(15));

      status.write();

      switch (tan.get() & 0b111)
      {
        case 0:
          ehc.write(0, IPC_ReadAdcBufferSum(16));
          ehc.write(1, IPC_ReadAdcBufferSum(17));
          ehc.write(2, IPC_ReadAdcBufferSum(18));
          break;
        case 1:
          ehc.write(3, IPC_ReadAdcBufferSum(19));
          ehc.write(4, IPC_ReadAdcBufferSum(20));
          ehc.write(5, IPC_ReadAdcBufferSum(21));
          break;
        case 2:
          ehc.write(6, IPC_ReadAdcBufferSum(22));
          ehc.write(7, IPC_ReadAdcBufferSum(23));
          ihc.write(0, IPC_ReadAdcBufferSum(24));
          break;
        case 3:
          ihc.write(1, IPC_ReadAdcBufferSum(25));
          ihc.write(2, IPC_ReadAdcBufferSum(26));
          ihc.write(3, IPC_ReadAdcBufferSum(27));
          break;
        case 4:
          ali.write(0, IPC_ReadAdcBufferSum(28));
          ali.write(1, IPC_ReadAdcBufferSum(29));
          psu.write(0, IPC_ReadAdcBufferSum(30));
          break;
        case 5:
          psu.write(1, IPC_ReadAdcBufferSum(31));
          rotenc.write();
          buttons.write();
          break;
        case 6:
          spare.write(0);
          spare.write(1);
          spare.write(2);
          break;
        case 7:
          spare.write(3);
          spare.write(4);
          spare.write(5);
          break;
      }
      LED_ERROR = (USB_MIDI_Send(0, (uint8_t *) (&UsbMidiSysex::sensorAndStatusData), sizeof UsbMidiSysex::sensorAndStatusData) == -1);
    }
  };

}
