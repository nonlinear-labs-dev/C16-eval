#pragma once

#include "tasks/mtask.h"
#include "drv/IoPin.h"
#include "io/pins.h"


static inline uint32_t readButtons(void)
{
  return 0b1111 ^ (pinBUTTON_D << 3 | pinBUTTON_C << 2 | pinBUTTON_B << 1 | pinBUTTON_A << 0);
}

#define ehcSetup_Default(channel)  pinEHC_PU_##channel = 1, pinEHC_PD_##channel = 1, pinEHC_nSER_##channel = !1
#define ehcSetup_Unloaded(channel) pinEHC_PU_##channel = 0, pinEHC_PD_##channel = 0, pinEHC_nSER_##channel = !1
#define ehcSetup_Pullup(channel)   pinEHC_PU_##channel = 1, pinEHC_PD_##channel = 0, pinEHC_nSER_##channel = !0
#define ehcSetup_CVm5p5(channel)   pinEHC_PU_##channel = 1, pinEHC_PD_##channel = 0, pinEHC_nSER_##channel = !1
#define ehcSetup_CV0p10(channel)   pinEHC_PU_##channel = 0, pinEHC_PD_##channel = 1, pinEHC_nSER_##channel = !1

static inline void test(void)
{
#if 0
  ehcSetup_Unloaded(1); ehcSetup_Pullup(0);
  ehcSetup_Unloaded(3); ehcSetup_Pullup(2);
  ehcSetup_Unloaded(5); ehcSetup_Pullup(4);
  ehcSetup_Unloaded(7); ehcSetup_Pullup(6);
#else
  ehcSetup_Unloaded(0);
  ehcSetup_Unloaded(1);
  ehcSetup_Unloaded(2);
  ehcSetup_Unloaded(3);
  ehcSetup_Unloaded(4);
  ehcSetup_Unloaded(5);
  ehcSetup_Unloaded(6);
  ehcSetup_Unloaded(7);
#endif
}

namespace Task
{

  class AllIoPins : public Task::Task
  {
    using Task::Task;

   public:
    IOpins::IOpin m_LED_m4HeartBeat { pinLED_M4HB };
    IOpins::IOpin m_LED_warning { pinLED_WARNING };
    IOpins::IOpin m_LED_error { pinLED_ERROR };
    IOpins::IOpin m_LED_keybedEvent { pinLED_E };
    IOpins::IOpin m_LED_uartActivity { pinLED_F };
    IOpins::IOpin m_LED_uartError { pinLED_J };
    IOpins::IOpin m_LED_usbDelayedPacket { pinLED_H };
    IOpins::IOpin m_LED_adcOverrun { pinLED_I };

    inline void body(void)
    {
      // m_LED_m4HeartBeat.process();  // not used
      m_LED_warning.process();
      m_LED_error.process();
      m_LED_keybedEvent.process();
      m_LED_uartActivity.process();
      m_LED_uartError.process();
      m_LED_usbDelayedPacket.process();
      m_LED_adcOverrun.process();

      test();
    };
  };
}
