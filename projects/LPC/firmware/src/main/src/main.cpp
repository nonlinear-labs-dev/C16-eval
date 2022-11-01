#include "tasks/createDataObjects.h"
#include "CPU_clock.h"
#include "drv/nl_cgu.h"
#include "ipc/ipc.h"
#include "cr_start_m0.h"
#include "drv/uart/uartHardware.h"

#include "usb/driver/nl_usb_midi.h"
#include "usb/driver/nl_usb_descmidi.h"

#if 0

// optimization test code
class IOPin
{
 private:
  typedef uint32_t volatile t_PinMemoryMapped;
  t_PinMemoryMapped &m_ioPinRef;

 public:
  constexpr IOPin(t_PinMemoryMapped &ioPin)
      : m_ioPinRef(ioPin) {};

  inline void set(uint32_t const flag) const
  {
    m_ioPinRef = flag;
  }

  inline void toggle(void) const
  {
    m_ioPinRef = ~m_ioPinRef;
  }

  inline uint32_t get(void) const
  {
    return m_ioPinRef;
  }
};

class myIOPins
{
 private:
  IOPin m_pin;

 public:
  typedef struct
  {
    volatile uint32_t W;
  } GPIO_PORT_Type;

  constexpr myIOPins(void)
      : m_pin(((GPIO_PORT_Type *) 0x400f502c)->W) {};

  void doWork(void)
  {
    m_pin.toggle();
  }
};

class Scheduler
{
 private:
  myIOPins m_myIoPins;

 public:
  constexpr Scheduler(void)
      : m_myIoPins() {};

  void doStuff(void)
  {
    m_myIoPins.doWork();
  }
};

// this version (data on stack) optimizes correctly
static void test1(void)
{
  Scheduler sched;
  asm volatile("mov r5, r5");
  sched.doStuff();
}

// but this version (static data) does not (memory still addressed indirectly)
Scheduler sched;
static void test2(void)
{
  asm volatile("mov r4, r4");
  sched.doStuff();
}

#endif

static inline void HardwareAndLowLevelInit(void);
static inline void M4SysTick_Init(void);

static Task::TaskScheduler *pScheduler;

// ---------------
int main(void)
{
  HardwareAndLowLevelInit();

  //test1();
  //test2();

  static Task::TaskScheduler scheduler;  // alas, data on stack doesn't solve the problem as expected from the above
  pScheduler = &scheduler;

  M4SysTick_Init();
  while (1)
  {
    scheduler.run();
  }
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
  UART_Init();

  cr_start_m0(&__core_m0app_START__);

  USB_MIDI_Config(0, Receive_IRQ_DummyCallback);
  USB_MIDI_Config(1, Receive_IRQ_DummyCallback);
  USB_MIDI_SetupDescriptors();
  USB_MIDI_Init(0);
  USB_MIDI_Init(1);

  pinLED_ERROR   = 0;
  pinLED_WARNING = 0;
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
  uint32_t timesliceTicker5us = ++s.timesliceTicker5us;
  switch (timesliceTicker5us)  // 25 * 5us   = 125us time slice)
  {
    case 3:
    case 9:
    case 15:
    case 21:  // runs at 4x speed, and values chosen to afford good interleaving with the dispatcher
      pScheduler->runTIRQ4x();
      break;

    case 25:
      s.timesliceTicker5us = 0u;
      pScheduler->dispatch();
      break;
  }
}
