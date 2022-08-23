#include "io/pins.h"
#include "ipc/ipc.h"

int main(void)
{
  while (1)
  {
    LED_M0HB = ~LED_M4HB;
  }
  return 0;
}
