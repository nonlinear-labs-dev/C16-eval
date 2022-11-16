// ------- UART (Serial Interface) Driver, Rx and Tx Callbacks
//  Rx Callback contains the message parser

#pragma once

#include <stdint.h>
#include "drv/IoPin.h"
#include "drv/EHC.h"
#include "usb/usbWriter_HardwareAccess.h"
#include "tasks/lraTask.h"
#include "uartProtocolDefs.h"
#include "uartProtocol.h"
#include "uartMessageComposer.h"

namespace UartProtocol
{

  static IOpins::IOpin *                pUartActivityLED;
  static IOpins::IOpin *                pUartErrorLED;
  static Task::LRAHandler *             pLraHandler;
  static UartProtocol::MessageComposer *pMsgComposer;
  static UsbWriter::HardwareAccess *    pUsbHw0;
  static UsbWriter::HardwareAccess *    pUsbHw1;

  inline static void setLeds(IOpins::IOpin &UartActivityLED, IOpins::IOpin &UartErrorLED)
  {
    pUartActivityLED = &UartActivityLED;
    pUartErrorLED    = &UartErrorLED;
  };

  inline static void setLraHandler(Task::LRAHandler &LraHandler)
  {
    pLraHandler = &LraHandler;
  }

  inline static void setMsgComposer(UartProtocol::MessageComposer &msgComposer)
  {
    pMsgComposer = &msgComposer;
  }

  inline static void setUsbHw(UsbWriter::HardwareAccess &hw0, UsbWriter::HardwareAccess &hw1)
  {
    pUsbHw0 = &hw0;
    pUsbHw1 = &hw1;
  }

  // Message Parser for Playcontroller side
  static void uartReceiveComplete_Callback(uint8_t contentId, uint8_t len, uint8_t *pData)
  {
    uint8_t tanHi, tanLo;
    do  // { ... } while (0), for using "continue" rather than "goto"
    {
      switch ((enum UartProtocol::MessageIds) contentId)
      {
        default:
          break;

        case UartProtocol::MessageIds::EhcSetup:
          if (len != 2 + 3)
            break;
          tanHi = pData[0];
          tanLo = pData[1];
          EhcHardware::configure(pData[2], pData[3], pData[4]);
          pMsgComposer->sendAck(tanHi, tanLo);
          continue;

        case UartProtocol::MessageIds::LraControl:
          if (len != 2 + 1)
            break;
          tanHi = pData[0];
          tanLo = pData[1];
          pLraHandler->startPattern(pData[2]);
          pMsgComposer->sendAck(tanHi, tanLo);
          continue;

        case UartProtocol::MessageIds::UsbControl:
          if (len != 2 + 2)
            break;
          tanHi = pData[0];
          tanLo = pData[1];
          pUsbHw0->config(pData[2]);
          pUsbHw1->config(pData[3]);
          pMsgComposer->sendAck(tanHi, tanLo);
          continue;
      }

      pUartErrorLED->timedOn(4);
      return;
    } while (0);

    pUartActivityLED->timedOn(4);
  };

  // hardware access for transmitting a byte
  static int uartTransmitCallback(uint8_t const byte, uint32_t const lineStatus)
  {
    if (!UART_ReadyToSend(lineStatus))
      return 0;

    UART_SendByte(byte);
    return 1;
  }

}  // namespace
