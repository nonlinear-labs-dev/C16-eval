#pragma once

#include "ipc/ipc.h"
#include "tasks/mtask.h"
#include "usb/nl_usb_midi.h"
#include "io/pins.h"

namespace Task
{

#warning temp
  // clang-format off
  static uint8_t midiData[36] =
  {
    0x14, 0xF0, 0x00, 0x00,  // 0
	0x14, 0x01, 0x01, 0x02,  // 4
	0x14, 0x02, 0x03, 0x03,  // 8
	0x14, 0x04, 0x04, 0x05,  // 12
	0x14, 0x05, 0x06, 0x06,  // 16
	0x14, 0x07, 0x07, 0x08,  // 20
	0x14, 0x08, 0x09, 0x09,  // 24
	0x14, 0x10, 0x10, 0x11,  // 28
	0x16, 0x11, 0xF7, 0x00,  // 32
  };
  // clang-format on

  class Adc : public Task::Task
  {
    using Task::Task;

   private:
    union
    {
      struct
      {
        struct
        {
          int32_t w0;
          int32_t w1;
        } erp[8];
        int32_t ehc[8];
        int32_t aftertouch;
        int32_t bender;
        int32_t ribbon[2];
        int32_t light[2];
        int32_t mon19V;
        int32_t mon5V;

      } data;
      int32_t array[32];
    } ch;

   public:
    // no dispatcher and body needed
    inline void dispatch(void)
    {
    }

    inline void body(void)
    {
    }

    // we check for the signal from M0 core here and run the job directly
    inline void run(void)
    {
      if (s.adcCycleFinished)
      {
        __disable_irq();
        s.adcCycleFinished = 0;
        LED_E              = ~LED_E;
        __enable_irq();

        for (unsigned i = 0; i < IPC_ADC_NUMBER_OF_CHANNELS; i++)
          ch.array[i] = IPC_ReadAdcBufferSum(i);

#warning temp
        midiData[2]  = (ch.data.erp[0].w0 & 0b11111110000000) >> 7;
        midiData[3]  = ch.data.erp[0].w0 & 0b1111111;
        midiData[5]  = (ch.data.erp[0].w1 & 0b11111110000000) >> 7;
        midiData[6]  = ch.data.erp[0].w1 & 0b1111111;
        midiData[7]  = (ch.data.erp[1].w0 & 0b11111110000000) >> 7;
        midiData[9]  = ch.data.erp[1].w0 & 0b1111111;
        midiData[10] = (ch.data.erp[1].w1 & 0b11111110000000) >> 7;
        midiData[11] = ch.data.erp[1].w1 & 0b1111111;
        midiData[13] = (ch.data.erp[2].w0 & 0b11111110000000) >> 7;
        midiData[14] = ch.data.erp[2].w0 & 0b1111111;
        midiData[15] = (ch.data.erp[2].w1 & 0b11111110000000) >> 7;
        midiData[17] = ch.data.erp[2].w1 & 0b1111111;
        midiData[18] = (ch.data.erp[3].w0 & 0b11111110000000) >> 7;
        midiData[19] = ch.data.erp[3].w0 & 0b1111111;
        midiData[21] = (ch.data.erp[3].w1 & 0b11111110000000) >> 7;
        midiData[22] = ch.data.erp[3].w1 & 0b1111111;
        midiData[23] = (ch.data.erp[4].w0 & 0b11111110000000) >> 7;
        midiData[25] = ch.data.erp[4].w0 & 0b1111111;
        midiData[26] = (ch.data.erp[4].w1 & 0b11111110000000) >> 7;
        midiData[27] = ch.data.erp[4].w1 & 0b1111111;
        midiData[29] = (ch.data.erp[5].w0 & 0b11111110000000) >> 7;
        midiData[30] = ch.data.erp[5].w0 & 0b1111111;
        midiData[31] = (ch.data.erp[5].w1 & 0b11111110000000) >> 7;
        midiData[33] = ch.data.erp[5].w1 & 0b1111111;
        LED_ERROR    = (USB_MIDI_Send(0, midiData, sizeof midiData) == -1);
      }
    }
  };

}
