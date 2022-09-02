#pragma once

#include <stdint.h>

namespace UsbMidiSysex
{
  static constexpr unsigned SENSOR_DATA_CABLE_NUMBER = 1u;
  static constexpr unsigned SENSOR_DATA_SYSEX3_SC    = (SENSOR_DATA_CABLE_NUMBER << 4) | 0x04;
  static constexpr unsigned SENSOR_DATA_SYSEX1_END   = (SENSOR_DATA_CABLE_NUMBER << 4) | 0x05;
  static constexpr unsigned SENSOR_DATA_SYSEX2_END   = (SENSOR_DATA_CABLE_NUMBER << 4) | 0x06;
  static constexpr unsigned SENSOR_DATA_SYSEX3_END   = (SENSOR_DATA_CABLE_NUMBER << 4) | 0x07;

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
	uint8_t erp0Hi;
	uint8_t erp0Lo;
	uint8_t erp1Hi;

    uint8_t cnci_3;
	uint8_t erp1Lo;
	uint8_t erp2Hi;
	uint8_t erp2Lo;

    uint8_t cnci_4;
	uint8_t erp3Hi;
	uint8_t erp3Lo;
	uint8_t erp4Hi;

    uint8_t cnci_5;
	uint8_t erp4Lo;
	uint8_t erp5Hi;
	uint8_t erp5Lo;

	uint8_t cnci_6;
    uint8_t mcmd_2;
    uint8_t dummy1;
    uint8_t dummy2;
  } sensorAndStatusData =
  {
	SENSOR_DATA_SYSEX3_SC,  0xF0, 0x00, 0x00,
    SENSOR_DATA_SYSEX3_SC,  0x00, 0x00, 0x00,
    SENSOR_DATA_SYSEX3_SC,  0x00, 0x00, 0x00,
    SENSOR_DATA_SYSEX3_SC,  0x00, 0x00, 0x00,
    SENSOR_DATA_SYSEX3_SC,  0x00, 0x00, 0x00,
	SENSOR_DATA_SYSEX1_END, 0xF7, 0x00, 0x00

  };
  // clang-format off

  struct Tan
  {
    void write(unsigned const value)
    {
      sensorAndStatusData.tanHi = getSysexHiByte(value);
      sensorAndStatusData.tanLo = getSysexLoByte(value);
    }

    unsigned next(unsigned const value)
    {
      if (value == 16383)
    	  return 1;
      return value + 1;
    }
  };

  struct Erp
  {
    void write(unsigned const erpNumber, unsigned const wiper1, unsigned const wiper2)
    {
      unsigned angle = wiper1;
      switch (erpNumber)
      {
        case 0 :
          sensorAndStatusData.erp0Hi = getSysexHiByte(angle);
          sensorAndStatusData.erp0Lo = getSysexLoByte(angle);
          return;
        case 1 :
          sensorAndStatusData.erp1Hi = getSysexHiByte(angle);
          sensorAndStatusData.erp1Lo = getSysexLoByte(angle);
          return;
        case 2 :
          sensorAndStatusData.erp2Hi = getSysexHiByte(angle);
          sensorAndStatusData.erp2Lo = getSysexLoByte(angle);
          return;
        case 3 :
          sensorAndStatusData.erp3Hi = getSysexHiByte(angle);
          sensorAndStatusData.erp3Lo = getSysexLoByte(angle);
          return;
        case 4 :
          sensorAndStatusData.erp4Hi = getSysexHiByte(angle);
          sensorAndStatusData.erp4Lo = getSysexLoByte(angle);
          return;
        case 5 :
          sensorAndStatusData.erp5Hi = getSysexHiByte(angle);
          sensorAndStatusData.erp5Lo = getSysexLoByte(angle);
          return;
      }
    }
  };

}
