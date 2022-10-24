#pragma once

#include <stdint.h>
#include "drv/IoPin.h"
#include "drv/EHC.h"
#include "tasks/lraTask.h"

namespace UartReceiveParser
{

  IOpins::IOpin *   pUartActivityLED;
  IOpins::IOpin *   pUartErrorLED;
  Task::LRAHandler *pLraHandler;

  enum ContentIds
  {
    EhcSetup   = 'E',
    LraControl = 'L',
  };

  inline static void setLeds(IOpins::IOpin &UartActivityLED, IOpins::IOpin &UartErrorLED)
  {
    pUartActivityLED = &UartActivityLED;
    pUartErrorLED    = &UartErrorLED;
  };

  inline static void setLraHandler(Task::LRAHandler &LraHandler)
  {
    pLraHandler = &LraHandler;
  }

  static void uartReceiveComplete_Callback(uint8_t contentId, uint8_t len, uint8_t *pData)
  {
    do  // { ... } while (0), for using "continue" rather than "goto"
    {
      switch ((enum ContentIds) contentId)
      {
        default:
          break;

        case EhcSetup:
          if (len != 3)
            break;
          EhcHardware::configure(pData[0], pData[1], pData[2]);
          continue;

        case LraControl:
          if (len != 1)
            break;
          pLraHandler->startPattern(pData[0]);
          continue;
      }

      pUartErrorLED->timedOn(4);
      return;
    } while (0);

    pUartActivityLED->timedOn(4);
  };

}  // namespace
