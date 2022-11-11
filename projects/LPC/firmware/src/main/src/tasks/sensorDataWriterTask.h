#pragma once

#include "drv/IoPin.h"
#include "ipc/ipc.h"
#include "tasks/mtask.h"
#include "tasks/encoderTask.h"
#include "usb/usbWriters.h"
#include "tasks/statemonitor.h"
#include "erp/ERP_Decoder.h"

namespace Task
{
  static constexpr unsigned SENSOR_DATA_CABLE_NUMBER = 2u;
  static constexpr unsigned TWENTYONE_ONES           = 0b111111111111111111111;  // 3*7 ones DO NOT CHANGE
  static constexpr unsigned FOURTEEN_ONES            = 0b000000011111111111111;  // 2*7 ones DO NOT CHANGE
  static constexpr unsigned FAULT_ANGLE              = TWENTYONE_ONES;

  class SensorDataWriter : public Task::Task
  {
    using Task::Task;

   private:
    unsigned                    m_tan { 0 };
    Usb::UsbMidiSysexWriter&    m_sensorEventWriter;
    StateMonitor::StateMonitor& m_stateMonitor;
    Encoder&                    m_encoder;

   public:
    constexpr SensorDataWriter(uint32_t const delay, uint32_t const period,
                               Usb::UsbMidiSysexWriter&    sensorEventWriter,
                               StateMonitor::StateMonitor& stateMonitor,
                               Encoder&                    encoder)
        : Task(delay, period)
        , m_sensorEventWriter(sensorEventWriter)
        , m_stateMonitor(stateMonitor)
        , m_encoder(encoder) {};

   private:
    inline unsigned erpWipersToAngle(unsigned const wiper0, unsigned const wiper1)
    {
      int angleCandidate = ERP_DecodeWipersToAngle(wiper1, wiper0);
      if (angleCandidate == ERP_INT_MAX)
        angleCandidate = FAULT_ANGLE;
      return (unsigned) angleCandidate;
    }

    void inline writeErp(unsigned const erpNumber)
    {
      unsigned angle = erpWipersToAngle(IPC_ReadAdcBufferSum(erpNumber * 2), IPC_ReadAdcBufferSum(erpNumber * 2 + 1));
      m_sensorEventWriter.write(SENSOR_DATA_CABLE_NUMBER,
                                Usb::getSysexHi2Byte(angle),
                                Usb::getSysexHiByte(angle),
                                Usb::getSysexLoByte(angle));
    };

   public:
    inline void body(void)
    {
      if (s.adcIsConverting)
        m_stateMonitor.event(StateMonitor::ERROR_ADC_OVERRUN);

      // We can restart the ADC immediately without risk of it overwriting values during collection
      // because it takes ~20us before the first value in the ADC value array is updated
      // and by that time this collecting function has finished (as takes ~10us).
      s.adcIsConverting = 1;

      // tan
      m_tan = (m_tan + 1u) & FOURTEEN_ONES;

      if (!m_sensorEventWriter.claimBufferElements(13))  // 13 4-byte frames not available ?
      {
        m_stateMonitor.event(StateMonitor::ERROR_SENSOR_DATA_LOSS);
        return;
      }

      m_sensorEventWriter.write(SENSOR_DATA_CABLE_NUMBER,
                                0xF0,  // SysEx Start
                                Usb::getSysexHiByte(m_tan),
                                Usb::getSysexLoByte(m_tan));

      // 8x ERP
      writeErp(0);
      writeErp(1);
      writeErp(2);
      writeErp(3);
      writeErp(4);
      writeErp(5);
      writeErp(6);
      writeErp(7);

      // status
      m_stateMonitor.setEhcDet();
      uint32_t status = m_stateMonitor.getStatus();

      m_sensorEventWriter.write(SENSOR_DATA_CABLE_NUMBER,
                                Usb::getSysexHi3Byte(status),  // stat0 hi/lo, stat1 hi
                                Usb::getSysexHi2Byte(status),
                                Usb::getSysexHiByte(status));

      // Low Speed Data
      uint32_t lsd0 = 0;
      uint32_t lsd1 = 0;
      uint32_t lsd2 = 0;

      switch (m_tan & 0b111)
      {
        case 0:
          lsd0 = IPC_ReadAdcBufferSum(ADC_EHC0);
          lsd1 = IPC_ReadAdcBufferSum(ADC_EHC1);
          lsd2 = IPC_ReadAdcBufferSum(ADC_EHC2);
          break;
        case 1:
          lsd0 = IPC_ReadAdcBufferSum(ADC_EHC3);
          lsd1 = IPC_ReadAdcBufferSum(ADC_EHC4);
          lsd2 = IPC_ReadAdcBufferSum(ADC_EHC5);
          break;
        case 2:
          lsd0 = IPC_ReadAdcBufferSum(ADC_EHC6);
          lsd1 = IPC_ReadAdcBufferSum(ADC_EHC0);
          lsd2 = IPC_ReadAdcBufferSum(ADC_AT);
          break;
        case 3:
          lsd0 = IPC_ReadAdcBufferSum(ADC_PB);
          lsd1 = IPC_ReadAdcBufferSum(ADC_RIB1);
          lsd2 = IPC_ReadAdcBufferSum(ADC_RIB2);
          break;
        case 4:
          lsd0 = IPC_ReadAdcBufferSum(ADC_AMBLI1);
          lsd1 = IPC_ReadAdcBufferSum(ADC_AMBLI2);
          lsd2 = IPC_ReadAdcBufferSum(ADC_PSU_19V);
          break;
        case 5:
          lsd0 = IPC_ReadAdcBufferSum(ADC_PSU_5V);
          lsd1 = uint32_t(m_encoder.getAndClearEncoderDelta());
          lsd2 = IOpins::readButtons();
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

      m_sensorEventWriter.write(SENSOR_DATA_CABLE_NUMBER,
                                Usb::getSysexLoByte(status),
                                Usb::getSysexHiByte(lsd0),
                                Usb::getSysexLoByte(lsd0));  // stat1 lo, lsd0 hi/lo
      m_sensorEventWriter.write(SENSOR_DATA_CABLE_NUMBER,
                                Usb::getSysexHiByte(lsd1),
                                Usb::getSysexLoByte(lsd1),
                                Usb::getSysexHiByte(lsd2));  // lsd1 hi/lo, lsd2 hi
      m_sensorEventWriter.writeLast(SENSOR_DATA_CABLE_NUMBER,
                                    Usb::getSysexLoByte(lsd2),
                                    0xF7);  // SysEx End
    }

  };  // class SensorDataWriter

}  // namespace
