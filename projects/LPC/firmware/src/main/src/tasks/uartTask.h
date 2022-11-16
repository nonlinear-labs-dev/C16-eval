#pragma once

#include "tasks/mtask.h"
#include "tasks/lraTask.h"
#include "drv/IoPin.h"
#include "drv/uart/uartHardware.h"
#include "drv/uart/uartProtocol.h"
#include "drv/uart/uartCallbacks.h"
#include "drv/uart/uartMessageComposer.h"
#include "usb/usbWriter_HardwareAccess.h"

namespace Task
{
  class Uart : public Task::Task
  {
   private:
    IOpins::IOpin&                m_uartActivityLED;
    IOpins::IOpin&                m_uartErrorLED;
    uint32_t                      m_lineStatus { 0 };
    UartProtocol::TxAssembler     m_txAssembler { UartProtocol::uartTransmitCallback };
    UartProtocol::MessageComposer m_msgComposer { m_txAssembler };
    UartProtocol::RxParser        m_rxParser { UartProtocol::uartReceiveComplete_Callback };

   public:
    Uart(IOpins::IOpin& uartActivityLED, IOpins::IOpin& uartErrorLED, LRAHandler& lraHandler, UsbWriter::HardwareAccess& usbHw0, UsbWriter::HardwareAccess& usbHw1)
        : Task()
        , m_uartActivityLED(uartActivityLED)
        , m_uartErrorLED(uartErrorLED)
    {
      UartProtocol::setLeds(m_uartActivityLED, m_uartErrorLED);
      UartProtocol::setLraHandler(lraHandler);
      UartProtocol::setUsbHw(usbHw0, usbHw1);
      UartProtocol::setMsgComposer(m_msgComposer);
    };

    // no dispatcher and body needed
    inline void dispatch(void) {};
    inline void body(void) {};

    inline void run(void)
    {
      while (1)
      {
        m_lineStatus = UART_GetAndClearLineStatusRegister();

        m_txAssembler.processPendingTransmits(m_lineStatus);

        if (UART_ReceiveError(m_lineStatus))
        {
          m_rxParser.onByteReceived(-1);         // force abort...
          (void) m_rxParser.getAndClearError();  // ...but clear resulting error
          m_uartErrorLED.timedOn(10);
          return;
        }
        if (!UART_DataAvailable(m_lineStatus))
          break;
        m_uartActivityLED.timedOn(1);
        m_rxParser.onByteReceived(UART_ReceiveByte());
        if (m_rxParser.getAndClearError() != UartProtocol::RxParser::Errors::None)
          m_uartErrorLED.timedOn(10);
      }
    }

  };  // class Uart

}
