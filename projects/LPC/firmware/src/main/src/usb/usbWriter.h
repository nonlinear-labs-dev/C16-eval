#pragma once

#include <stdint.h>
#include "tasks/mtask.h"
#include "sys/ticker.h"

namespace UsbWriter
{

  enum class USBPorts
  {
    USB0 = 0,
    USB1 = 1,
  };

  typedef uint64_t tTime;

  static constexpr tTime TIMEOUT_TIMEOUT_INITIAL    = msToTicks(100);
  static constexpr tTime TIMEOUT_TIMEOUT_SUBSEQUENT = msToTicks(10);

  // ---- virtual base class ----
  class UsbWriter
  {
   private:
    virtual bool waitingForDataReady(void)           = 0;
    virtual bool waitingForTransmitStarted(void)     = 0;
    virtual bool waitingForTransmitDone(void)        = 0;
    virtual bool processOnlyOneTransmitAtATime(void) = 0;
    virtual void killTransfer(void)                  = 0;

    inline bool timedOut(void)
    {
      if ((ticker - m_packetTime) > m_packetTimeout)
      {
        m_dropped = true;
        m_state   = States::IDLE;
        killTransfer();
        return true;
      }
      return false;
    };

   protected:
    inline void stateMachine(void)
    {
      while (1)
      {
        switch (m_state)
        {
          case States::IDLE:
            if (waitingForDataReady())
              return;
            m_packetTimeout = !m_dropped ? TIMEOUT_TIMEOUT_INITIAL : TIMEOUT_TIMEOUT_SUBSEQUENT;
            m_dropped       = false;
            m_packetTime    = ticker;
            m_state         = States::DATA_READY;
            // intentional fall-through

          case States::DATA_READY:
            if (waitingForTransmitStarted() || timedOut())
              return;
            m_state = States::TRANSMITTING;
            // intentional fall-through

          case States::TRANSMITTING:
            if (waitingForTransmitDone() || timedOut())
              return;
            m_state = States::IDLE;
            if (processOnlyOneTransmitAtATime())
              return;
            // intentional fall-through to IDLE
        }
      }
    };

    inline bool isIdle(void)
    {
      return m_state == States::IDLE;
    }

    inline void resetTransfer(void)
    {
      m_state         = States::IDLE;
      m_dropped       = false;
      m_packetTimeout = TIMEOUT_TIMEOUT_INITIAL;
    }

    // ---- data members ----
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
    tTime m_packetTime { 0 };
    tTime m_packetTimeout { TIMEOUT_TIMEOUT_INITIAL };
    bool  m_dropped { false };
  };

}  // namespace Usb
