#pragma once

#include <stdint.h>
#include "usbWriter_HardwareAccess.h"

namespace UsbWriter
{
  class WriterStateMachine
  {
   private:
    virtual bool waitingForUserDataReady(void) const  // user class must implement this
    {
      return false;
    };

    virtual void userTransactionPreStart(void) {};  // user class may shadow this

    virtual void userTransactionPostStart(void) {};  // user class may shadow this

    virtual void finishUserTransaction(void) {};  // user class may shadow this

    virtual void abortUserTransaction(void) {};  // user class may shadow this

    virtual bool processOnlyOneTransmitAtATime(void) const  // user class may shadow this
    {
      return true;
    };

   public:
    constexpr WriterStateMachine(HardwareAccess &hwAccess)
        : m_hwAccess(hwAccess) {};

    enum class USBTimeouts
    {
      UseTimeout,
      DontUseTimeout,
    };

    void setupTransmitData(void *const pData, uint16_t const dataSize, enum USBTimeouts const timeout) const
    {
      m_hwAccess.setupTransactionData(pData, dataSize, timeout == USBTimeouts::UseTimeout);
    };

    void process(void)
    {
      do
      {
        switch (m_state)
        {
          case States::IDLE:
            if (waitingForUserDataReady())
              return;
            if (!m_hwAccess.prepareTransaction())
              return;  // hardware-driver still busy
            m_state = States::DATA_READY;
            // intentional fall-through

          case States::DATA_READY:
            if (timeOut())
              continue;  // check for new data after abort
            userTransactionPreStart();
            if (!m_hwAccess.startTransaction())
              return;  // try again next time
            userTransactionPostStart();
            m_state = States::TRANSMITTING;
            // intentional fall-through

          case States::TRANSMITTING:
            if (timeOut())
              continue;  // check for new data after abort
            if (!m_hwAccess.transactionFinished())
              return;  // still sending
            finishUserTransaction();
            m_state = States::IDLE;
            if (processOnlyOneTransmitAtATime())
              return;
            // intentional fall-through
        }
      } while (1);
    };

   private:
    bool timeOut(void)
    {
      if (m_hwAccess.timedOut())
      {
        m_hwAccess.killTransaction();
        m_state = States::IDLE;
        abortUserTransaction();
        return true;
      }
      return false;
    };

    // ---- data members ----
   protected:
    HardwareAccess &m_hwAccess;

   private:
    enum class States
    {
      IDLE,
      DATA_READY,
      TRANSMITTING,
    };
    enum States m_state
    {
      States::IDLE
    };

  };  // class WriterStateMachine

}  // namespace UsbWriter
