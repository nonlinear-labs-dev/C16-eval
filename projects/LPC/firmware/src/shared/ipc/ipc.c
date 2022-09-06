#include <stdint.h>
#include "ipc.h"

//
// Places the variable(s) in the shared memory block.
// This block must be defined in C15_LPC43xx_MEM.ld.
// The section used must be defined in both C15_LPC43xx_M0.ld and C15_LPC43xx_M4.ld identically
// and must be defined properly and identically in the *_MEM.ld script includes.
// When the size of all shared variables is larger than the block the linker will throw an error.
// To make sure all variable are at the same memory location and have the same layout,
// use only one single struct and embed all your variables there.
// The section is defined as "noinit" so you must clear the data yourself.
//
__attribute__((section(".noinit.$RamAHB_ETB16"))) SharedData_T s;

// double-check the used memory at compile time
static inline void CheckSizeAtCompileTime(void)
{
  (void) sizeof(char[-!(sizeof(s) < 3 * 4096 + 2000)]);  // must be less than this # of bytes in shared mem
}
