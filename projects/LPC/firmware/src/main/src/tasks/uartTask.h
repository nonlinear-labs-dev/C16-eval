#pragma once

#include "tasks/mtask.h"
#include "drv/IoPin.h"
#include "drv/uart/uartHardware.h"
#include "drv/uart/uartProtocol.h"

namespace Task
{
  static void uartReceiveComplete_Callback(uint8_t contentId, uint8_t len, uint8_t* pData)
  {
	  // TODO : Message Parser/Handler
  }

  class Uart : public Task::Task
  {
   private:
    IOpins::IOpin&         m_uartActivityLED;
    IOpins::IOpin&         m_uartErrorLED;
    uint32_t               m_lineStatus { 0 };
    UartProtocol::RxParser m_rxParser { uartReceiveComplete_Callback };

   public:
    Uart(IOpins::IOpin& uartActivityLED, IOpins::IOpin& uartErrorLED)
        : Task()
        , m_uartActivityLED(uartActivityLED)
        , m_uartErrorLED(uartErrorLED) {};

    Uart(uint32_t const delay, uint32_t const period,
         IOpins::IOpin& uartActivityLED, IOpins::IOpin& uartErrorLED)
        : Task(delay, period)
        , m_uartActivityLED(uartActivityLED)
        , m_uartErrorLED(uartErrorLED) {};

    inline void body(void)
    {
      while (1)
      {
        m_lineStatus = UART_GetAndClearLineStatusRegister();
        if (UART_ReceiveError(m_lineStatus))
        {
          m_rxParser.onByteReceived(-1);         // force abort...
          (void) m_rxParser.getAndClearError();  // ...but clear resulting error
          m_uartErrorLED.timedOn(10);
          return;
        }
        if (!UART_DataAvailable(m_lineStatus))
          break;
        m_rxParser.onByteReceived(UART_ReceiveByte());
        if (m_rxParser.getAndClearError() != UartProtocol::RxParser::Errors::None)
          m_uartErrorLED.timedOn(10);
        m_uartActivityLED.timedOn(1);
      }
    }

  };  // class Uart

}
