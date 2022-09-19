#include "tasks/createDataObjects.h"
#include "CPU_clock.h"
#include "drv/nl_cgu.h"
#include "ipc/ipc.h"
#include "cr_start_m0.h"

#include "usb/driver/nl_usb_midi.h"
#include "usb/driver/nl_usb_descmidi.h"

static inline void HardwareAndLowLevelInit(void);

static Task::TaskScheduler scheduler;

// ---------------
int main(void)
{
  HardwareAndLowLevelInit();
  while (1)
    scheduler.run();
  return 0;
}

// ----------------
static inline void M4SysTick_Init(void);

static void Receive_IRQ_DummyCallback(uint8_t const port, uint8_t *buff, uint32_t len)
{
}

static inline void HardwareAndLowLevelInit(void)
{
  CPU_ConfigureClocks();
  IPC_Init();

  M4SysTick_Init();
  cr_start_m0(&__core_m0app_START__);

  USB_MIDI_Config(0, Receive_IRQ_DummyCallback);
  USB_MIDI_Config(1, Receive_IRQ_DummyCallback);
  USB_MIDI_SetupDescriptors();
  USB_MIDI_Init(0);
  USB_MIDI_Init(1);

  LED_ERROR   = 0;
  LED_WARNING = 0;
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
  s.ticker5us++;
  if (++s.timesliceTicker5us == 25u)  // 25 * 5us   = 125us time slice)
  {
    s.timesliceTicker5us = 0u;
    scheduler.dispatch();
  }
}
