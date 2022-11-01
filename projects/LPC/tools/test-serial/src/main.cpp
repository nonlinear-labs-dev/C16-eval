// C library headers
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

// Linux headers
#include <fcntl.h>            // Contains file controls like O_RDWR
#include <errno.h>            // Error integer and strerror() function
#include <termios.h>          // Contains POSIX terminal control definitions
#include <unistd.h>           // write(), read(), close()
#include <sys/ioctl.h>        //ioctl() call defenitions
#include <linux/tty_flags.h>  // for ASYNC_LOW_LATENCY

#include "uart/uartProtocol.h"
#include "uart/uartProtocolDefs.h"
#include "LRApatternIds.h"

static void setupPort(int const portFd)
{
  // clear RTS and DTR
  int data = TIOCM_RTS;
  ioctl(portFd, TIOCMBIC, &data);
  data = TIOCM_DTR;
  ioctl(portFd, TIOCMBIC, &data);

  // low latency
  ioctl(portFd, TIOCGSERIAL, &data);
  data |= ASYNC_LOW_LATENCY;
  ioctl(portFd, TIOCSSERIAL, &data);

  struct termios tty;
  if (tcgetattr(portFd, &tty) != 0)
    exit(3);

  tty.c_cflag &= ~PARENB;         // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB;         // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag &= ~CSIZE;          // Clear all bits that set the data size
  tty.c_cflag |= CS8;             // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS;        // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL;  // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO;                                                         // Disable echo
  tty.c_lflag &= ~ECHOE;                                                        // Disable erasure
  tty.c_lflag &= ~ECHONL;                                                       // Disable new-line echo
  tty.c_lflag &= ~ISIG;                                                         // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                       // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);  // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST;  // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR;  // Prevent conversion of newline to carriage return/line feed

  tty.c_cc[VTIME] = 0;  // non-blocking...
  tty.c_cc[VMIN]  = 0;  //...reads

  // Set in/out baud rate to 115200
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);

  // Save tty settings, also checking for error
  if (tcsetattr(portFd, TCSANOW, &tty) != 0)
    exit(3);
}

static void prompt(char const* const pString)
{
  if (pString)
    printf("%s", pString);
  char*   line     = nullptr;
  size_t  len      = 0;
  ssize_t lineSize = getline(&line, &len, stdin);
  free(line);
}

int main(void)
{
  int portFd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
  if (portFd == -1)
    return 3;

  setupPort(portFd);

  prompt("Start LPC Debugger (if required) and press Enter...");

  uint8_t sendMsg[] = {
    UartProtocol::MAGIC0,
    UartProtocol::MAGIC1,
    UartProtocol::MAGIC2,
    UartProtocol::MAGIC3,
    uint8_t(UartProtocol::MessageIds::LraControl),
    3,     // msgSize
    0x1B,  // tanHi
    0x1B,  // tanHi (doubled)
    0x22,  // tanLo
    0x0C,  // lraCtrl
  };

  if (write(portFd, sendMsg, sizeof sendMsg) == -1)
    return 3;

  unsigned cnt = 0;
  while (cnt < 4 + 1 + 1 + 2 + 1)
  {
    ssize_t ret;
    uint8_t byte;
    do
      ret = read(portFd, &byte, 1);
    while (ret != 1);
    printf("%02X ", byte);
    fflush(stdout);
    cnt++;
  }
  printf("\n");
  fflush(stdout);

  prompt("press Enter to terminate...");

  close(portFd);
  return 0;
}
