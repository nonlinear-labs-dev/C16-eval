#pragma once

#include "tasks/mtask.h"
#include "drv/IoPin.h"
#include "sys/uart.h"

namespace Task
{

  class UartEchoTest : public Task::Task
  {
   private:
    IOpins::IOpin& m_uartActivityLED;
    IOpins::IOpin& m_uartErrorLED;
    uint32_t       m_lineStatus = 0;

   public:
    constexpr UartEchoTest(IOpins::IOpin& uartActivityLED, IOpins::IOpin& uartErrorLED)
        : Task()
        , m_uartActivityLED(uartActivityLED)
        , m_uartErrorLED(uartErrorLED) {};

    constexpr UartEchoTest(uint32_t const delay, uint32_t const period,
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
          m_uartErrorLED.timedOn(10);
          return;
        }
        if (!UART_DataAvailable(m_lineStatus))
          break;
        UART_SendByte(UART_ReceiveByte());
        m_uartActivityLED.timedOn(1);
      }
    }
  };

}
