#pragma once

#include <stdint.h>
#include "usbWriter_HardwareAccess.h"

namespace UsbWriter
{
  class WriterStateMachine
  {
   private:
    virtual void onGoingOnline(void) {};  // user class may shadow this

    virtual bool waitingForUserDataReady(void) = 0;  // user class must implement this

    virtual void userTransactionPreStart(void) {};  // user class may shadow this

    virtual void userTransactionPostStart(void) {};  // user class may shadow this

    virtual void finishUserTransaction(void) {};  // user class may shadow this

    virtual void abortUserTransaction(void) {};  // user class may shadow this

    virtual bool processOnlyOneTransmitAtATime(void)  // user class may shadow this
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
      if (wentOnline())
        onGoingOnline();

      do
      {
        switch (m_state)
        {
          case States::IDLE:
            if (waitingForUserDataReady())
              return;
            if (m_hwAccess.isBusy())
              m_hwAccess.m_stateMonitor.event(StateMonitor::Events::WARNING_USB_DELAYED_PACKET);
            m_state = States::DRIVER_BUSY;
            // intentional fall-through

          case States::DRIVER_BUSY:
            if (!m_hwAccess.prepareTransaction())
              return;  // hardware-driver still busy
            m_state = States::DATA_READY;
            // intentional fall-through

          case States::DATA_READY:
            if (timeOut())
              return;  // check for new data after abort
            userTransactionPreStart();
            if (!m_hwAccess.startTransaction())
              return;  // try again next time
            userTransactionPostStart();
            m_state = States::TRANSMITTING;
            // intentional fall-through

          case States::TRANSMITTING:
            if (timeOut())
              return;  // check for new data after abort
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
    bool wentOnline(void)
    {
      bool online = m_hwAccess.isOnline();
      if (m_online != online)
      {
        m_online = online;
        return online;
      }
      return false;
    };

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
      DRIVER_BUSY,
      DATA_READY,
      TRANSMITTING,
    };
    enum States m_state
    {
      States::IDLE
    };

    uint8_t m_online { 0xAA };

  };  // class WriterStateMachine

}  // namespace UsbWriter
