#pragma once

namespace UartProtocol
{
  /*
   * General Message Format:
   *
   * Every message consists of a header, formed by 4 magic bytes, and a a body, containing the message ID,
   * the message length and the actual message payload (which can be zero length).
   *
   * The header is also used to reliably sync receiver to the transmitter when communication sets in
   * at an arbitrary point (or breaks and resumes).
   *
   * This is achieved by the special Escape pattern that is never appearing outside
   * of the header section because
   *  - outside of the header every ESC symbol is doubled,
   *  - message ID cannot be ESC, that is, there is a hole in the value space for message IDs, ranging from 0..254 only,
   *  - message size cannot be ESC. Values >= ESC are encoded as Value+1, again creating a hole in the value
   *    range, now spanning 0..254 only.
   *
   *
   * Logical Message Structure:
   *   ESC | SOH | ESC | STX | message-ID | payload-size (N) | [ payload byte 1 | ... | payload byte N ]
   *
   * In the physical message, every byte in the payload section that is equal to ESC has to be doubled,
   * for example | 19 | 1A | 1B | 1C | becomes | 19 | 1A | 1B | 1B | 1C |
   *
   */

  static constexpr uint8_t ESC = 0x1B;  // the escape symbol
  static constexpr uint8_t SOH = 0x01;  // start of header
  static constexpr uint8_t STX = 0x02;  // start of text
  static constexpr uint8_t ACK = 0x06;  // Acknowledge

  // magic header sequence
  static constexpr uint8_t MAGIC0 = ESC;
  static constexpr uint8_t MAGIC1 = SOH;
  static constexpr uint8_t MAGIC2 = ESC;
  static constexpr uint8_t MAGIC3 = STX;

  /*
   * Specific Message Bodies:
   *
   * 1. EHC Setup Messsage
   *
   *  All eight channels are configured at a time.
   *
   *  | EhcSetup ("E") | msg-len (5) | TAN-Hi | TAN-Lo | Series-Rs | PullUp-Rs | Pulldown-Rs |
   *
   *  TAN is a 16 bit transaction number which is used for an Acknowledge Message to be sent back to the transmitter.
   *
   *  Series-Rs, PullUp-Rs and Pulldown-Rs are bit fields, the MSB (Bit 7) corresponding to EHC channel 7,
   *  LSB corresponding to EHC channel 0.
   *
   *
   * 2. LRA Control Message
   *
   *  Only one of the eight LRAs can be controlled in a message.
   *
   *  | LraControl ("L") | msg-len (3) | TAN-Hi | TAN-Lo | lra-setup |

   *  TAN is a 16 bit transaction number which is used for an Acknowledge Message to be sent back to the transmitter.
   *
   *  lra-setup is a bitfield, bits 7..5 are the LRA number (0..7) and bits 4..0 are the selected LRA pattern
   *
   *
   * 3. Acknowledge Message
   *
   *  The messages above generate an Acknowledge message sent back to the transmitter, using the TAN as the payload
   *
   *  | ACK (0x06) | msg-len (2) | TAN-Hi | TAN-Lo |
   *
   *
   * 4. USB Control Message
   *
   *  Both USB ports are controlled in one message
   *
   *  | UsbControl ("U") | msg-len (4) | TAN-Hi | TAN-Lo | usb0-control | usb1-control
   *
   *  usb-control bit fields:
   *    bit 0 : enable USB
   *    bit 1 : clear USB flags
   *
   */

  enum class MessageIds
  {  // message ID == ESC (0x1B) is forbidden !
    EhcSetup    = 'E',
    LraControl  = 'L',
    UsbControl  = 'U',
    Acknowledge = ACK,
  };
}  // namespace
