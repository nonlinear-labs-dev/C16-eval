#pragma once

#include "io/pins.h"
#include "tasks/allIOpinsTask.h"

namespace StateMonitor
{
  enum Events
  {
    NO_EVENT = 0,
    ERROR_SENSOR_DATA_LOSS,
    ERROR_KEYBED_DATA_LOSS,
    ERROR_SENSOR_OR_KEYBED_DATA_LOSS,
    ERROR_BRIDGE_DATA_LOSS,
    WARNING_USB_DELAYED_PACKET,
    INFO_KEYBED_EVENT,
  };

  class StateMonitor
  {
   private:
    Task::AllIoPins &m_allIoPins;
    uint32_t         m_statusWord { 0 };

   public:
    StateMonitor(Task::AllIoPins &allIoPins)
        : m_allIoPins(allIoPins) {};

    inline uint32_t getStatus(void) const
    {
      return m_statusWord;
    };

    inline void event(enum Events event)
    {
      switch (event)
      {
        case NO_EVENT:
        default:
          break;

        case ERROR_KEYBED_DATA_LOSS:
        case ERROR_SENSOR_DATA_LOSS:
        case ERROR_SENSOR_OR_KEYBED_DATA_LOSS:
        case ERROR_BRIDGE_DATA_LOSS:
          m_allIoPins.m_LED_error.timedOn(10);
          break;

        case WARNING_USB_DELAYED_PACKET:
          m_allIoPins.m_LED_usbDelayedPacket.timedOn(2);
          break;

        case INFO_KEYBED_EVENT:
          m_allIoPins.m_LED_keybedEvent.timedOn(1);
      }
    };

    inline void setEhcDet(void)
    {  // pinEHC_DET_x must be bit access type
      uint32_t ehcDet = (pinEHC_DET_3 << 3 | pinEHC_DET_2 << 2 | pinEHC_DET_1 << 1 | pinEHC_DET_0 << 0);
      m_statusWord &= ~0x0F;
      m_statusWord |= ehcDet;
    }
  };

}  // namespace
