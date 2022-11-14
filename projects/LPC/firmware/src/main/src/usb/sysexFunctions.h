#pragma once

#include <stdint.h>

namespace SysEx
{

  static inline uint8_t getSysexHi4Byte(unsigned const value)
  {
    return (uint8_t)((value & 0b11110000000000000000000000000000) >> 28);
  }

  static inline uint8_t getSysexHi3Byte(unsigned const value)
  {
    return (value & 0b1111111000000000000000000000) >> 21;
  }

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

}  // namespace SysEx
