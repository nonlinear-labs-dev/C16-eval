#pragma once

#include "ipc/ipc.h"
#include "tasks/mtask.h"
#include "drv/iopins.h"
#include "usb/usb.h"
#include "erp/ERP_Decoder.h"

namespace Task
{

  static constexpr unsigned SENSOR_DATA_CABLE_NUMBER = 1u;

  static inline uint8_t getSysexHi2Byte(unsigned const value)
  {
    return (value & 0b111111100000000000000) >> 14;
  }

  static inline uint8_t getSysexHiByte(unsigned const value)
  {
    return (value & 0b11111110000000) >> 7;
  }

  static inline uint8_t getSysexLoByte(unsigned const value)
  {
    return (value & 0b1111111);
  }

  static inline unsigned erpWipersToAngle(unsigned const wiper0, unsigned const wiper1)
  {
    int angleCandidate = ERP_DecodeWipersToAngle(wiper1, wiper0);
    if (angleCandidate == ERP_INT_MAX)
      angleCandidate = 0b111111111111111111111;
    return (unsigned) angleCandidate;
  }

  class SensorDataWriter : public Task::Task
  {
    using Task::Task;

   private:
    unsigned tan;

    void inline writeErp(unsigned const erpNumber)
    {
      unsigned angle = erpWipersToAngle(IPC_ReadAdcBufferSum(erpNumber * 2), IPC_ReadAdcBufferSum(erpNumber * 2 + 1));
      sensorAndKeyEventWriter.write(SENSOR_DATA_CABLE_NUMBER, getSysexHi2Byte(angle), getSysexHiByte(angle), getSysexLoByte(angle));
    }

   public:
    inline void body(void)
    {
      sensorAndKeyEventWriter.start();

      // tan
      tan = (tan + 1u) & 0b11111111111111;
      sensorAndKeyEventWriter.write(SENSOR_DATA_CABLE_NUMBER, 0xF0, getSysexHiByte(tan), getSysexLoByte(tan));

      // 8x erp
      writeErp(0);
      writeErp(1);
      writeErp(2);
      writeErp(3);
      writeErp(4);
      writeErp(5);
      writeErp(6);
      writeErp(7);

      // status
      sensorAndKeyEventWriter.write(SENSOR_DATA_CABLE_NUMBER, 0, 0, 0);  // stat0 hi/lo, stat1 hi

      uint32_t lsd0 = 0;
      uint32_t lsd1 = 0;
      uint32_t lsd2 = 0;

      switch (tan & 0b111)
      {
        case 0:
          lsd0 = IPC_ReadAdcBufferSum(16);  // EHC0
          lsd1 = IPC_ReadAdcBufferSum(17);  // EHC1
          lsd2 = IPC_ReadAdcBufferSum(18);  // EHC2
          break;
        case 1:
          lsd0 = IPC_ReadAdcBufferSum(19);  // EHC3
          lsd1 = IPC_ReadAdcBufferSum(20);  // EHC4
          lsd2 = IPC_ReadAdcBufferSum(21);  // EHC5
          break;
        case 2:
          lsd0 = IPC_ReadAdcBufferSum(22);  // EHC6
          lsd1 = IPC_ReadAdcBufferSum(23);  // EHC7
          lsd2 = IPC_ReadAdcBufferSum(24);  // IHC0
          break;
        case 3:
          lsd0 = IPC_ReadAdcBufferSum(25);  // IHC1
          lsd1 = IPC_ReadAdcBufferSum(26);  // IHC2
          lsd2 = IPC_ReadAdcBufferSum(27);  // IHC3
          break;
        case 4:
          lsd0 = IPC_ReadAdcBufferSum(28);  // ALI0
          lsd1 = IPC_ReadAdcBufferSum(29);  // ALI1
          lsd2 = IPC_ReadAdcBufferSum(30);  // PSU0
          break;
        case 5:
          lsd0 = IPC_ReadAdcBufferSum(31);  // PSU1
          lsd1 = 0;                         // ROTENC
          lsd2 = 0;                         // BUTTONS
          break;
        case 6:
          lsd0 = 0;  // SPARE0
          lsd1 = 0;  // SPARE1
          lsd2 = 0;  // SPARE2
          break;
        case 7:
          lsd0 = 0;  // SPARE3
          lsd1 = 0;  // SPARE4
          lsd2 = 0;  // SPARE5
          break;
      }

      sensorAndKeyEventWriter.write(SENSOR_DATA_CABLE_NUMBER, 0, getSysexHiByte(lsd0), getSysexLoByte(lsd0));                     // stat1 lo, lsd0 hi/lo
      sensorAndKeyEventWriter.write(SENSOR_DATA_CABLE_NUMBER, getSysexHiByte(lsd1), getSysexLoByte(lsd1), getSysexHiByte(lsd2));  // lsd1 hi/lo, lsd2 hi
      sensorAndKeyEventWriter.writeLast(SENSOR_DATA_CABLE_NUMBER, getSysexLoByte(lsd2), 0xF7);
      if (sensorAndKeyEventWriter.send() == -1)
        LED_USBstalling.timedOn(3);
    }
  };

}
