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
    INFO_USB_WENT_ONLINE,
    INFO_USB_WENT_OFFLINE,
    INFO_USB_PACKET_DELIVERED,
    INFO_USB_PACKET_DROPPED,
    INFO_USB_IS_ONLINE,
    INFO_USB_IS_OFFLINE,
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

    inline void event(enum Events const event)
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
          break;
      }
    };

    inline void event(enum Events const event, uint8_t const usbPort)
    {
      uint8_t maskOffset = usbPort ? 4 + 5 : 4;
      switch (event)
      {
        default:
          break;

        case INFO_USB_WENT_ONLINE:
          m_statusWord |= (1 << (0 + maskOffset));  // bits 4, 9
          break;

        case INFO_USB_WENT_OFFLINE:
          m_statusWord |= (1 << (1 + maskOffset));  // bits 5, 10
          break;

        case INFO_USB_PACKET_DELIVERED:
          m_statusWord |= (1 << (2 + maskOffset));  // bits 6, 11
          break;

        case INFO_USB_PACKET_DROPPED:
          m_statusWord |= (1 << (3 + maskOffset));  // bits 7, 12
          break;

        case INFO_USB_IS_ONLINE:
          m_statusWord |= (1 << (4 + maskOffset));  // bits 8, 13
          break;

        case INFO_USB_IS_OFFLINE:
          m_statusWord &= ~(1 << (4 + maskOffset));  // bits 8, 13
          break;
      }
    };

    inline void setEhcDet(void)
    {  // pinEHC_DET_x must be bit access type
      uint32_t ehcDet = (pinEHC_DET_3 << 3 | pinEHC_DET_2 << 2 | pinEHC_DET_1 << 1 | pinEHC_DET_0 << 0);
      m_statusWord &= ~0x0F;
      m_statusWord |= ehcDet;
    };

    inline void clearUsbFlags(uint8_t const port)
    {
      if (port)
        m_statusWord &= ~(0b11111 << (4 + 5));
      else
        m_statusWord &= ~(0b11111 << 4);
    };
  };

}  // namespace
