#include "drv/rit.h"
#include "io/pins.h"
#include "ipc/ipc.h"
#include "drv/kbs.h"
#include "drv/adc.h"

int main(void)
{
  KBS_Init();
  RIT_Init_IntervalInHz(M0_IRQ_FREQ_HZ);

  while (1)
  {
    LED_M0HB = ~LED_M4HB;
    processADCs();
  }
  return 0;
}

static inline uint32_t M4TicksToUS(uint32_t const ticks)
{
  return M4_PERIOD_US * ticks;
}

static uint32_t rand32 = 0x12345678;

static inline uint32_t xorshift_u32(void)
{  // 32 bit xor-shift generator, period 2^32-1, non-zero seed required, range 1...2^32-1
   // source : https://www.jstatsoft.org/index.php/jss/article/view/v008i14/xorshift.pdf
  rand32 ^= rand32 << 13;
  rand32 ^= rand32 >> 17;
  rand32 ^= rand32 << 5;
  return rand32;
}

// ---------------- clock jitter for timer IRQ
static inline void jitterIRQ(void)
{
  RIT_SetCompVal((uint32_t)((NL_LPC_CLK / M0_IRQ_FREQ_HZ) - 32u + (xorshift_u32() & 63u)));  // offset clock by 2^6(+-32) clock periods
}

/******************************************************************************/
void M0_RIT_OR_WWDT_IRQHandler(void)
{
  // Clear interrupt flag early, to allow for short, single cycle IRQ overruns,
  // that is if, the keybed scanner occasionally takes longer than one time slot
  // (but no more than two), the interrupt routine is invoked again as soon as
  // it completed the first pass. By this, no IRQ is lost, albeit the ticker
  // used in KBS_Process for key timing will have a little bit of jitter.
  // Only when eeprom memory access etc slows down the bus there is a slight
  // chance of a burst of overruns where the M0 main process hardly gets any
  //  execution time if the IRQ execution time is close to IRQ repeat rate already.
  RIT_ClearInt();
  jitterIRQ();
  (*KBS_Process)();
  s.M0_KbsIrqOvers += (1 & RIT_GetCtrlReg());  // note that this profiling increases IRQ time itself (Heisenberg ;-)
}
