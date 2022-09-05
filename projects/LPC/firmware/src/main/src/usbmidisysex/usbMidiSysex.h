#pragma once

#include <stdint.h>
#include "erp/ERP_Decoder.h"

#define ERP67isERP0W01 (1)  // copy raw wiper values or ERP0 to ERP6 und 7

#if ERP67isERP0W01
#warning special erp6 raw wiper values are copied to erp0.w0 and erp7 is erp0.w1
#endif

namespace UsbMidiSysex
{
  static constexpr unsigned SENSOR_DATA_CABLE_NUMBER = 1u;
  static constexpr unsigned SENSOR_DATA_SYSEX3_SC    = (SENSOR_DATA_CABLE_NUMBER << 4) | 0x04;
  static constexpr unsigned SENSOR_DATA_SYSEX1_END   = (SENSOR_DATA_CABLE_NUMBER << 4) | 0x05;
  static constexpr unsigned SENSOR_DATA_SYSEX2_END   = (SENSOR_DATA_CABLE_NUMBER << 4) | 0x06;
  static constexpr unsigned SENSOR_DATA_SYSEX3_END   = (SENSOR_DATA_CABLE_NUMBER << 4) | 0x07;

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

  // clang-format off
  static struct __attribute__((packed))
  {
    uint8_t cnci_1;
    uint8_t mcmd_1;
    uint8_t tanHi;
    uint8_t tanLo;

    uint8_t cnci_2;
	uint8_t erp0Hi2;
	uint8_t erp0Hi;
	uint8_t erp0Lo;

    uint8_t cnci_3;
	uint8_t erp1Hi2;
	uint8_t erp1Hi;
	uint8_t erp1Lo;

    uint8_t cnci_4;
	uint8_t erp2Hi2;
	uint8_t erp2Hi;
	uint8_t erp2Lo;

    uint8_t cnci_5;
	uint8_t erp3Hi2;
	uint8_t erp3Hi;
	uint8_t erp3Lo;

    uint8_t cnci_6;
	uint8_t erp4Hi2;
	uint8_t erp4Hi;
	uint8_t erp4Lo;

    uint8_t cnci_7;
	uint8_t erp5Hi2;
	uint8_t erp5Hi;
	uint8_t erp5Lo;

    uint8_t cnci_8;
	uint8_t erp6Hi2;
	uint8_t erp6Hi;
	uint8_t erp6Lo;

    uint8_t cnci_9;
	uint8_t erp7Hi2;
	uint8_t erp7Hi;
	uint8_t erp7Lo;


	uint8_t cnci_10;
    uint8_t stat0Hi;
    uint8_t stat0Lo;
    uint8_t stat1Hi;

	uint8_t cnci_11;
    uint8_t stat1Lo;
    union
	{
      uint8_t ehc0;
      uint8_t ehc3;
      uint8_t ehc6;
      uint8_t ihc1;
      uint8_t ali0;
      uint8_t psu1;
      uint8_t spare0;
      uint8_t spare3;
	} lsd0Hi;
	union
	{
	  uint8_t ehc0;
	  uint8_t ehc3;
	  uint8_t ehc6;
	  uint8_t ihc1;
	  uint8_t ali0;
	  uint8_t psu1;
	  uint8_t spare0;
	  uint8_t spare3;
	} lsd0Lo;

	uint8_t cnci_12;
	union
	{
	  uint8_t ehc1;
	  uint8_t ehc4;
	  uint8_t ehc7;
	  uint8_t ihc2;
	  uint8_t ali1;
	  uint8_t rotenc;
	  uint8_t spare1;
	  uint8_t spare4;
	} lsd1Hi;
	union
	{
	  uint8_t ehc1;
	  uint8_t ehc4;
	  uint8_t ehc7;
	  uint8_t ihc2;
	  uint8_t ali1;
	  uint8_t rotenc;
	  uint8_t spare1;
	  uint8_t spare4;
	} lsd1Lo;
	union
	{
	  uint8_t ehc2;
	  uint8_t ehc5;
	  uint8_t ihc0;
	  uint8_t ihc3;
	  uint8_t psu0;
	  uint8_t buttons;
	  uint8_t spare2;
	  uint8_t spare5;
	} lsd2Hi;

	uint8_t cnci_13;
	union
	{
	  uint8_t ehc2;
	  uint8_t ehc5;
	  uint8_t ihc0;
	  uint8_t ihc3;
	  uint8_t psu0;
	  uint8_t buttons;
	  uint8_t spare2;
	  uint8_t spare5;
	} lsd2Lo;
    uint8_t mcmd_2;
    uint8_t dummy;
  } sensorAndStatusData =
  {
	SENSOR_DATA_SYSEX3_SC,  0xF0, 0xA0, 0xA0,
    SENSOR_DATA_SYSEX3_SC,  0xE0, 0xE0, 0xE0,
    SENSOR_DATA_SYSEX3_SC,  0xE1, 0xE1, 0xE1,
    SENSOR_DATA_SYSEX3_SC,  0xE2, 0xE2, 0xE2,
    SENSOR_DATA_SYSEX3_SC,  0xE3, 0xE3, 0xE3,
    SENSOR_DATA_SYSEX3_SC,  0xE4, 0xE4, 0xE4,
    SENSOR_DATA_SYSEX3_SC,  0xE5, 0xE5, 0xE5,
    SENSOR_DATA_SYSEX3_SC,  0xE6, 0xE6, 0xE6,
    SENSOR_DATA_SYSEX3_SC,  0xE7, 0xE7, 0xE7,
	SENSOR_DATA_SYSEX3_SC,  0xA0, 0xA0, 0xA1,
	SENSOR_DATA_SYSEX3_SC,  0xA1, 0xD0, 0xD0,
	SENSOR_DATA_SYSEX3_SC,  0xD1, 0xD1, 0xD2,
	SENSOR_DATA_SYSEX2_END, 0xD2, 0xF7, 0x00
  };
  // clang-format on

  //
  // Transaction Number
  //
  struct Tan
  {
   private:
    unsigned m_transactionNumber = 0u;

   public:
    inline void write(void)
    {
      sensorAndStatusData.tanHi = getSysexHiByte(m_transactionNumber);
      sensorAndStatusData.tanLo = getSysexLoByte(m_transactionNumber);
    }

    inline void next(void)
    {
      m_transactionNumber = (m_transactionNumber + 1u) & 0b11111111111111;
    }

    inline unsigned get(void)
    {
      return m_transactionNumber;
    }
  };

  //
  // ERP 0..7 (Endless Rotary Potentiometers)
  //
  struct Erp
  {
    inline void write(unsigned const erpNumber, unsigned const wiper0, unsigned const wiper1)
    {
      int angleCandidate = ERP_DecodeWipersToAngle(wiper1, wiper0);
      if (angleCandidate == ERP_INT_MAX)
        angleCandidate = 0b111111111111111111111;
      unsigned angle = (unsigned) angleCandidate;

      switch (erpNumber)
      {
        case 0:
          sensorAndStatusData.erp0Hi2 = getSysexHi2Byte(angle);
          sensorAndStatusData.erp0Hi  = getSysexHiByte(angle);
          sensorAndStatusData.erp0Lo  = getSysexLoByte(angle);
#if (ERP67isERP0W01)
          sensorAndStatusData.erp6Hi2 = 0;
          sensorAndStatusData.erp6Hi  = getSysexHiByte(wiper0);
          sensorAndStatusData.erp6Lo  = getSysexLoByte(wiper0);
          sensorAndStatusData.erp7Hi2 = 0;
          sensorAndStatusData.erp7Hi  = getSysexHiByte(wiper1);
          sensorAndStatusData.erp7Lo  = getSysexLoByte(wiper1);
#endif
          return;
        case 1:
          sensorAndStatusData.erp1Hi2 = getSysexHi2Byte(angle);
          sensorAndStatusData.erp1Hi  = getSysexHiByte(angle);
          sensorAndStatusData.erp1Lo  = getSysexLoByte(angle);
          return;
        case 2:
          sensorAndStatusData.erp2Hi2 = getSysexHi2Byte(angle);
          sensorAndStatusData.erp2Hi  = getSysexHiByte(angle);
          sensorAndStatusData.erp2Lo  = getSysexLoByte(angle);
          return;
        case 3:
          sensorAndStatusData.erp3Hi2 = getSysexHi2Byte(angle);
          sensorAndStatusData.erp3Hi  = getSysexHiByte(angle);
          sensorAndStatusData.erp3Lo  = getSysexLoByte(angle);
          return;
        case 4:
          sensorAndStatusData.erp4Hi2 = getSysexHi2Byte(angle);
          sensorAndStatusData.erp4Hi  = getSysexHiByte(angle);
          sensorAndStatusData.erp4Lo  = getSysexLoByte(angle);
          return;
        case 5:
          sensorAndStatusData.erp5Hi2 = getSysexHi2Byte(angle);
          sensorAndStatusData.erp5Hi  = getSysexHiByte(angle);
          sensorAndStatusData.erp5Lo  = getSysexLoByte(angle);
          return;
#if !(ERP67isERP0W01)
        case 6:
          sensorAndStatusData.erp6Hi2 = getSysexHi2Byte(angle);
          sensorAndStatusData.erp6Hi  = getSysexHiByte(angle);
          sensorAndStatusData.erp6Lo  = getSysexLoByte(angle);
          return;
        case 7:
          sensorAndStatusData.erp7Hi2 = getSysexHi2Byte(angle);
          sensorAndStatusData.erp7Hi  = getSysexHiByte(angle);
          sensorAndStatusData.erp7Lo  = getSysexLoByte(angle);
          return;
#endif
      }
    }
  };

  //
  // status flags etc
  //
  struct Status
  {
    inline void write(void)
    {
      // TODO fill with real data
      sensorAndStatusData.stat0Hi = 0;
      sensorAndStatusData.stat0Lo = 0;
      sensorAndStatusData.stat1Hi = 0;
      sensorAndStatusData.stat1Lo = 0;
    }
  };

  //
  // EHC 0..7 (External Hardware Controllers)
  //
  struct Ehc
  {
    inline void write(unsigned const ehcNumber, unsigned const value)
    {
      switch (ehcNumber)
      {
        case 0:
          sensorAndStatusData.lsd0Hi.ehc0 = getSysexHiByte(value);
          sensorAndStatusData.lsd0Lo.ehc0 = getSysexLoByte(value);
          return;
        case 1:
          sensorAndStatusData.lsd1Hi.ehc1 = getSysexHiByte(value);
          sensorAndStatusData.lsd1Lo.ehc1 = getSysexLoByte(value);
          return;
        case 2:
          sensorAndStatusData.lsd2Hi.ehc2 = getSysexHiByte(value);
          sensorAndStatusData.lsd2Lo.ehc2 = getSysexLoByte(value);
          return;
        case 3:
          sensorAndStatusData.lsd0Hi.ehc3 = getSysexHiByte(value);
          sensorAndStatusData.lsd0Lo.ehc3 = getSysexLoByte(value);
          return;
        case 4:
          sensorAndStatusData.lsd1Hi.ehc4 = getSysexHiByte(value);
          sensorAndStatusData.lsd1Lo.ehc4 = getSysexLoByte(value);
          return;
        case 5:
          sensorAndStatusData.lsd2Hi.ehc5 = getSysexHiByte(value);
          sensorAndStatusData.lsd2Lo.ehc5 = getSysexLoByte(value);
          return;
        case 6:
          sensorAndStatusData.lsd0Hi.ehc6 = getSysexHiByte(value);
          sensorAndStatusData.lsd0Lo.ehc6 = getSysexLoByte(value);
          return;
        case 7:
          sensorAndStatusData.lsd1Hi.ehc7 = getSysexHiByte(value);
          sensorAndStatusData.lsd1Lo.ehc7 = getSysexLoByte(value);
          return;
      }
    }
  };

  //
  // IHC 0..3 (Internal Hardware Controllers)
  //
  struct Ihc
  {
    inline void write(unsigned const ihcNumber, unsigned const value)
    {
      switch (ihcNumber)
      {
        case 0:
          sensorAndStatusData.lsd2Hi.ihc0 = getSysexHiByte(value);
          sensorAndStatusData.lsd2Lo.ihc0 = getSysexLoByte(value);
          return;
        case 1:
          sensorAndStatusData.lsd0Hi.ihc1 = getSysexHiByte(value);
          sensorAndStatusData.lsd0Lo.ihc1 = getSysexLoByte(value);
          return;
        case 2:
          sensorAndStatusData.lsd1Hi.ihc2 = getSysexHiByte(value);
          sensorAndStatusData.lsd1Lo.ihc2 = getSysexLoByte(value);
          return;
        case 3:
          sensorAndStatusData.lsd2Hi.ihc3 = getSysexHiByte(value);
          sensorAndStatusData.lsd2Lo.ihc3 = getSysexLoByte(value);
          return;
      }
    }
  };

  //
  // ALI 0..1 (Ambient Light)
  //
  struct Ali
  {
    inline void write(unsigned const aliNumber, unsigned const value)
    {
      switch (aliNumber)
      {
        case 0:
          sensorAndStatusData.lsd0Hi.ali0 = getSysexHiByte(value);
          sensorAndStatusData.lsd0Lo.ali0 = getSysexLoByte(value);
          return;
        case 1:
          sensorAndStatusData.lsd1Hi.ali1 = getSysexHiByte(value);
          sensorAndStatusData.lsd1Lo.ali1 = getSysexLoByte(value);
          return;
      }
    }
  };

  //
  // PSU 0..1 (PSU Voltages 0 and 1)
  //
  struct Psu
  {
    inline void write(unsigned const psuNumber, unsigned const value)
    {
      switch (psuNumber)
      {
        case 0:
          sensorAndStatusData.lsd2Hi.psu0 = getSysexHiByte(value);
          sensorAndStatusData.lsd2Lo.psu0 = getSysexLoByte(value);
          return;
        case 1:
          sensorAndStatusData.lsd0Hi.psu1 = getSysexHiByte(value);
          sensorAndStatusData.lsd0Lo.psu1 = getSysexLoByte(value);
          return;
      }
    }
  };

  //
  // ROTENC (Rotary Encoder)
  //
  struct RotEnc
  {
    inline void write()
    {
      // TODO fill with real data
      sensorAndStatusData.lsd1Hi.rotenc = 0;
      sensorAndStatusData.lsd1Lo.rotenc = 0;
    }
  };

  //
  // Buttons
  //
  struct Buttons
  {
    inline void write()
    {
      // TODO fill with real data
      sensorAndStatusData.lsd2Hi.buttons = 0;
      sensorAndStatusData.lsd2Lo.buttons = 0;
    }
  };

  //
  // SPARE 0..6 (spare/future data)
  //
  struct Spare
  {
    inline void write(unsigned const spareNumber)
    {
      switch (spareNumber)
      {
        case 0:
          sensorAndStatusData.lsd0Hi.spare0 = 0;
          sensorAndStatusData.lsd0Lo.spare0 = 0;
          return;
        case 1:
          sensorAndStatusData.lsd1Hi.spare1 = 0;
          sensorAndStatusData.lsd1Lo.spare1 = 0;
          return;
        case 2:
          sensorAndStatusData.lsd2Hi.spare2 = 0;
          sensorAndStatusData.lsd2Lo.spare2 = 0;
          return;
        case 3:
          sensorAndStatusData.lsd0Hi.spare3 = 0;
          sensorAndStatusData.lsd0Lo.spare3 = 0;
          return;
        case 4:
          sensorAndStatusData.lsd1Hi.spare4 = 0;
          sensorAndStatusData.lsd1Lo.spare4 = 0;
          return;
        case 5:
          sensorAndStatusData.lsd2Hi.spare5 = 0;
          sensorAndStatusData.lsd2Lo.spare5 = 0;
          return;
      }
    }
  };

}  // namespace
