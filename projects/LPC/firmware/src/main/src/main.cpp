#include "cr_start_m0.h"
#include "io/pins.h"
#include "CPU_clock.h"
#include "drv/nl_cgu.h"
#include "ipc/ipc.h"
#include "tasks/tasks.h"

static inline void Init(void);

// ---------------
int main(void)
{
  Init();

  while (1)
  {
    ledHeartBeatM4.run();
    ledErrorWarning.run();
    keybedScanner.run();
  }
  return 0;
}

// ----------------
static inline void M4SysTick_Init(void);

static inline void Init(void)
{
  CPU_ConfigureClocks();
  IPC_Init();
  cr_start_m0(&__core_m0app_START__);
  M4SysTick_Init();

  LED_WARNING = LED_ERROR = 0;
}

/*************************************************************************
 * @brief	ticker interrupt routines using the standard M4 system ticker
 *        IRQ will be triggered every 125us, our basic scheduler time-slice
 ******************************************************************************/
static inline void M4SysTick_Init(void)
{
#define SYST_CSR   (uint32_t *) (0xE000E010)  // SysTick Control & Status Reg
#define SYST_RVR   (uint32_t *) (0xE000E014)  // SysTick Reload Value
#define SYST_CVR   (uint32_t *) (0xE000E018)  // SysTick Counter Value
#define SYST_CALIB (uint32_t *) (0xE000E01C)  // SysTick Calibration
  *SYST_RVR = (NL_LPC_CLK / M4_FREQ_HZ) - 1;
  *SYST_CVR = 0;
  *SYST_CSR = 0b111;  // processor clock | IRQ enabled | counter enabled
}

extern "C" void SysTick_Handler(void)
{
  static unsigned cntr = 25u;  // 25 * 5us   = 125us time slice

  s.ticker++;
  if (!--cntr)
  {
    cntr = 25u;
    ledHeartBeatM4.dispatch();
    ledErrorWarning.dispatch();
    keybedScanner.dispatch();
  }
}
