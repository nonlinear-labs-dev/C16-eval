#pragma once

#include <drv/IoPin.h>
#include "ipc/ipc.h"
#include "tasks/mtask.h"
#include "usb/usb.h"
#include "erp/ERP_Decoder.h"

namespace Task
{
  static constexpr unsigned SENSOR_DATA_CABLE_NUMBER = 2u;

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
    unsigned                      tan;
    IOpins::IOpin&                m_adcOverrunLED;
    IOpins::IOpin&                m_dataLossLED;
    Usb::UsbMidiWriter_16kBuffer& m_sensorEventWriter;

    void inline writeErp(unsigned const erpNumber)
    {
      unsigned angle = erpWipersToAngle(IPC_ReadAdcBufferSum(erpNumber * 2), IPC_ReadAdcBufferSum(erpNumber * 2 + 1));
      m_sensorEventWriter.write(SENSOR_DATA_CABLE_NUMBER, Usb::getSysexHi2Byte(angle), Usb::getSysexHiByte(angle), Usb::getSysexLoByte(angle));
    }

   public:
    SensorDataWriter(uint32_t const delay, uint32_t const period, IOpins::IOpin& adcOverrunLED, IOpins::IOpin& dataLossLED, Usb::UsbMidiWriter_16kBuffer& sensorEventWriter)
        : Task(delay, period)
        , m_adcOverrunLED(dataLossLED)
        , m_dataLossLED(dataLossLED)
        , m_sensorEventWriter(sensorEventWriter) {};

    inline void body(void)
    {
      if (s.adcIsConverting)
        m_adcOverrunLED.set(1);

      // tan
      tan = (tan + 1u) & 0b11111111111111;

      if (m_sensorEventWriter.claimBuffer(13))  // 13 4-byte frames available ?
      {
        m_sensorEventWriter.write(SENSOR_DATA_CABLE_NUMBER, 0xF0, Usb::getSysexHiByte(tan), Usb::getSysexLoByte(tan));

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
        m_sensorEventWriter.write(SENSOR_DATA_CABLE_NUMBER, 0, 0, 0);  // stat0 hi/lo, stat1 hi

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

        DBG_TP1_0 = s.adcIsConverting = 1;

        m_sensorEventWriter.write(SENSOR_DATA_CABLE_NUMBER, 0, Usb::getSysexHiByte(lsd0), Usb::getSysexLoByte(lsd0));                          // stat1 lo, lsd0 hi/lo
        m_sensorEventWriter.write(SENSOR_DATA_CABLE_NUMBER, Usb::getSysexHiByte(lsd1), Usb::getSysexLoByte(lsd1), Usb::getSysexHiByte(lsd2));  // lsd1 hi/lo, lsd2 hi
        m_sensorEventWriter.writeLast(SENSOR_DATA_CABLE_NUMBER, Usb::getSysexLoByte(lsd2), 0xF7);
      }
      else  // Data Loss !!!
        m_dataLossLED.timedOn(1);
    }
  };

}
