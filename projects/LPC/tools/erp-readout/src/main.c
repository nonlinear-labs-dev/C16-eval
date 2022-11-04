#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <termios.h>
#include <sys/ioctl.h>

#define SHOW_RAW (0)

// qtcreator bugs
//#include </usr/include/stdarg.h>
#include <stdarg.h>
#undef NULL
#define NULL ((void *) 0)

#include "erp/ERP_Decoder.h"
#include "erp/ERP_Quantizer.h"

static struct ERP_Quantizer_t *quantizer;

#define PAYLOAD_BUFFER_SIZE (100000ul)

#define RAW_TX_BUFFER_SIZE (PAYLOAD_BUFFER_SIZE * 2ul)  // need headroom for 8-to-7 bit encoding of the payload
#define RAW_RX_BUFFER_SIZE (PAYLOAD_BUFFER_SIZE * 2ul)  // should have headroom for 8-to-7 bit encoding of the payload
#define PARSER_BUFFER_SIZE (RAW_RX_BUFFER_SIZE)

#define SEND_WAIT_MS (1ul)  // milliseconds to wait when send buffer is going to overrun

typedef enum
{
  FALSE = 0,
  TRUE
} BOOL;

static BOOL           send;   // flag for program function: 1-->send , 0-->receive
static char const *   pName;  // port name
static snd_rawmidi_t *port;   // MIDI port
static BOOL           stop;

static BOOL dump = FALSE;

int kbhit(void)
{
  struct termios original;
  tcgetattr(STDIN_FILENO, &original);

  struct termios term;
  memcpy(&term, &original, sizeof(term));

  term.c_lflag &= ~ICANON;
  tcsetattr(STDIN_FILENO, TCSANOW, &term);

  int characters_buffered = 0;
  ioctl(STDIN_FILENO, FIONREAD, &characters_buffered);

  tcsetattr(STDIN_FILENO, TCSANOW, &original);

  return (characters_buffered != 0);
}

static void error(char const *const format, ...)
{
  va_list ap;

  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  putc('\n', stderr);
}

static void cursorUp(uint8_t lines)
{
  char buffer[12];
  sprintf(buffer, "\033[%dA", lines);
  printf("%s", buffer);
}

static void usage(void)
{
  printf(
      "Usage: erp-readout -r port\n"
      "\n"
      "-r     : receive test data\n"
      "port   : MIDI port to test (in hw:x,y,z notation, see ouput of 'amidi -l'\n");
}

static inline void getCmdLineParams(int const argc, char const *const argv[])
{
  if (argc != 3)
  {
    error("missing parameters\n");
    usage();
    exit(1);
  }

  if (strlen(argv[1]) == 2 && 0 == strcmp(argv[1], "-s"))
    send = TRUE;
  else if (strlen(argv[1]) == 2 && 0 == strcmp(argv[1], "-r"))
    send = FALSE;
  else
  {
    error("illegal parameter '%s'\n", argv[1]);
    usage();
    exit(1);
  }

  pName = argv[2];
}

static inline void openPort(void)
{
  int err;
  if (send)
    err = snd_rawmidi_open(NULL, &port, pName, 0);  // send will be blocking
  else
    err = snd_rawmidi_open(&port, NULL, pName, SND_RAWMIDI_NONBLOCK);
  if (err < 0)
  {
    error("cannot open port \"%s\": %s", pName, snd_strerror(err));
    exit(1);
  }
}

static inline void closePort(void)
{
  snd_rawmidi_close(port);
}

static void sigHandler(int dummy)
{
  stop  = TRUE;
  dummy = 0;
}

static uint64_t getTimeUSec(void)
{
  struct timespec tp;

  if (clock_gettime(CLOCK_MONOTONIC, &tp))
    return 0;
  else
    return 1000000ul * (uint64_t)(tp.tv_sec) + (uint64_t)(tp.tv_nsec) / 1000ul;
}

//
// -------- functions for sending --------
//

static inline int encodeSysex(void const *const pSrc, int len, void *const pDest)
{
  uint8_t *src         = (uint8_t *) pSrc;
  uint8_t *dst         = (uint8_t *) pDest;
  uint8_t  topBitsMask = 0;
  uint8_t *topBits     = (uint8_t *) 0;

  // write start of sysex
  *(dst++) = 0xF0;

  while (len)
  {
    if (topBitsMask == 0)  // need to start insert a new top bits byte ?
    {
      topBitsMask = 0x40;   // reset top bit mask to first bit (bit6)
      topBits     = dst++;  // save top bits position...
      *topBits    = 0;      // ...and clear top bits
    }
    else  // regular byte
    {
      uint8_t byte = *(src++);
      *(dst++)     = byte & 0x7F;
      if (byte & 0x80)
        *topBits |= topBitsMask;  // add in top bit for this byte
      topBitsMask >>= 1;
      len--;
    }
  }

  // write end of sysex
  *(dst++) = 0xF7;
  return (void *) dst - pDest;  // total bytes written to destination buffer;
}

static inline void doSend(void)
{
  int     messageLen;
  uint8_t dataBuf[PAYLOAD_BUFFER_SIZE] __attribute__((aligned(8)));
  uint8_t sendBuf[RAW_TX_BUFFER_SIZE] __attribute__((aligned(8)));
  int     err, written;

  snd_rawmidi_status_t *pStatus;
  snd_rawmidi_status_alloca(&pStatus);

#if SHOW_RAW
  snd_rawmidi_params_t *pParams;
  snd_rawmidi_params_alloca(&pParams);

  if ((err = snd_rawmidi_params_set_buffer_size(port, pParams, sizeof sendBuf)) < 0)
  {
    error("cannot set send buffer size of %d : %s", sizeof sendBuf, snd_strerror(err));
    return;
  }
#endif

  if ((err = snd_rawmidi_nonblock(port, 0)) < 0)
  {
    error("cannot set blocking mode: %s", snd_strerror(err));
    return;
  }

  // messageLen = encodeSysex(dataBuf, PAYLOAD_SIZE, sendBuf);
  // printf("sysex size: %d\n", messageLen);
  uint8_t  runningCntr = 0;
  uint64_t messageNo   = 0;

  printf("Sending data to port: %s\n\n\n", pName);

  do
  {
    memset(dataBuf, runningCntr++, sizeof(dataBuf));
    if (runningCntr >= 128)
      runningCntr = 0;

    ((uint64_t *) dataBuf)[0] = getTimeUSec();
    ((uint64_t *) dataBuf)[1] = messageNo;

    unsigned expo = ((unsigned) rand()) % 11;  // 0..10
    unsigned size = 1 << expo;                 // 2^0...2^10(1024)
    size += ((unsigned) rand()) & (size - 1);  // 0..2047
    if (size & 1)
      size++;
    size += 24;
    ((uint64_t *) dataBuf)[2] = size;

    uint16_t *p   = (uint16_t *) &(dataBuf[24]);
    uint16_t  val = messageNo & 0xFFFF;
    for (unsigned i = 0; i < (size - 24) / 2; i++)
      *p++ = val++;

    cursorUp(1);
    printf("n:%" PRIu64 ", s:%5u\n", messageNo++, size);
    fflush(stdout);

    messageLen = encodeSysex(dataBuf, size, sendBuf);
    int total  = 0;

    uint64_t messageTime = getTimeUSec();
    while (total != messageLen)
    {
      int toTransfer = messageLen - total;

      if ((err = snd_rawmidi_status(port, pStatus)))
      {
        error("cannot get status: %s", snd_strerror(errno));
        return;
      }
      int avail = snd_rawmidi_status_get_avail(pStatus);
      if (0)  //toTransfer > avail)
      {
        error("rawmidi send larger than buffer by: %d", toTransfer - avail);
        usleep(1000ul * SEND_WAIT_MS);
        continue;
      }

      uint64_t time = getTimeUSec();
      written       = snd_rawmidi_write(port, &(sendBuf[total]), toTransfer);

      if ((err = snd_rawmidi_drain(port)) < 0)
      {
        error("cannot drain buffer: %s", snd_strerror(err));
        return;
      }
      if (written < 0)
      {
        error("cannot send data: %s", snd_strerror(written));
        return;
      }
      // printf("chunk of %d bytes transferred\n", written);

      total += written;
      time = getTimeUSec() - time;
      time *= 1 + (((unsigned) rand()) & 0xF);  // * 1..16
      //time /= 3;
      if (time > 1000ul * 1000ul)
        time = 1000ul * 1000ul;

      uint64_t sleepTime = getTimeUSec();
      usleep(time);
      sleepTime = getTimeUSec() - sleepTime;
      messageTime += sleepTime;
    }
    if (written != total)
    {
      ;  //printf("total of %d bytes transferred\n", total);
    }

    messageTime = getTimeUSec() - messageTime;

    static uint64_t min = ~(uint64_t) 0;
    static uint64_t max = 0;
    static uint32_t cnt = 0;
    static uint64_t sum = 0;

    if (messageTime > max)
      max = messageTime;
    if (messageTime < min)
      min = messageTime;
    sum += messageTime;
    cnt++;

    if (cnt == 1000)
    {
      cnt /= 2;
      sum /= 2;
    }
    if (cnt % 50 == 0)
    {
      cursorUp(2);
      printf("%6.2lfms(min) %6.1lfms(max) %6.1lfms(avg)\n\n",
             ((double) min) / 1000.0, ((double) max) / 1000.0, ((double) sum) / 1000.0 / cnt);
      fflush(stdout);
      max = max * 0.99;
      min = min / 0.80;
    }

  } while (TRUE);
}

//
// -------- functions for receiving --------
//

static int const TWENTYONE_ONES = 0b111111111111111111111;

static inline unsigned getPacketNr(uint8_t const *const data)
{
  return (uint32_t) data[0] * 128ul + (uint32_t) data[1];
}

static inline int getErpAngle(unsigned const erpNumber, uint8_t const *const data)
{
  int result = (int) data[2 + 3 * erpNumber] * 128u * 128u + data[3 + 3 * erpNumber] * 128u + data[4 + 3 * erpNumber];
  if (result == TWENTYONE_ONES)
    return result;
  if (result & 0b100000000000000000000)
    result |= 0b11111111111000000000000000000000;
  return result;
}

static inline BOOL examineContent(void const *const data, unsigned const len)
{
  static uint64_t time     = 0;
  uint64_t        now      = getTimeUSec();
  static double   maxTime  = 0.0;
  static double   minTime  = 1.0e9;
  static double   average  = 495.0;
  static int      period   = 500;
  static unsigned settling = 2000;
  static uint64_t bins[9];
  static uint64_t total;
  static uint32_t packetNr;
  static unsigned packetErrors = 0;
  static unsigned angleErrors  = 0;

  uint8_t const *const pErpData = data;

  if (kbhit())
  {
    int c = getchar();
    if (c == 'r')
    {
      settling = 1000;
      maxTime  = 0.0;
      minTime  = 1.0e9;
      average  = 495.0;
      total    = 0;
      for (int i = 0; i < 9; i++)
        bins[i] = 0;
    }
  }

  if (!settling)
  {
    int diff = (int) (now - time);
    if (diff > maxTime)
      maxTime = diff;
    if (diff < minTime)
      minTime = diff;
    average = average - (0.00001 * (average - diff));

    do
    {
      if (diff < period / 10)
      {
        bins[0]++;
        break;
      }
      if (diff < period / 3.16)
      {
        bins[1]++;
        break;
      }
      if (diff < period / 1.41)
      {
        bins[2]++;
        break;
      }
      if (diff < period / 1.122)
      {
        bins[3]++;
        break;
      }
      if (diff <= period * 1.122)
      {
        bins[4]++;
        break;
      }
      if (diff <= period * 1.41)
      {
        bins[5]++;
        break;
      }
      if (diff <= period * 3.16)
      {
        bins[6]++;
        break;
      }
      if (diff <= period * 10)
      {
        bins[7]++;
        break;
      }
      bins[8]++;
    } while (0);
    total++;
  }
  if (settling)
  {
    settling--;
    if (settling == 0)
      packetNr = getPacketNr(pErpData);
    else
      return TRUE;
  }
  else
  {
#if !SHOW_RAW
    printf("        %5.0lfus %7.2lfus %5.0lfus -- ", minTime, average, maxTime);
    for (int i = 0; i < 9; i++)
      printf("%8.4lf%c", 100.0 * (double) bins[i] / (double) total, bins[i] ? ' ' : 'z');
    printf("\n\033[1A");
#endif
  }
  time = now;

#define CHECK_SIZE (36)

  if (len != CHECK_SIZE)
  {
    error("receive: payload has wrong length %d, expected %d", len, CHECK_SIZE);
    return FALSE;
  }

  if (getPacketNr(pErpData) != packetNr)
    while (getPacketNr(pErpData) != packetNr)
    {
      packetErrors++;
      packetNr = (packetNr + 1) & 0b11111111111111;
    }
  packetNr = (getPacketNr(pErpData) + 1u) & 0b11111111111111;

  int      angleErp0 = getErpAngle(0, pErpData);
  int      angleErp1 = getErpAngle(1, pErpData);
  int      angleErp2 = getErpAngle(2, pErpData);
  int      angleErp3 = getErpAngle(3, pErpData);
  int      angleErp4 = getErpAngle(4, pErpData);
  int      angleErp5 = getErpAngle(5, pErpData);
  int      angleErp6 = getErpAngle(6, pErpData);
  int      angleErp7 = getErpAngle(7, pErpData);
  unsigned stat      = (unsigned) pErpData[26] * 128u * 128u * 128u + pErpData[27] * 128u * 128u + pErpData[28] * 128u + pErpData[29];
  int      lsd0      = (int) pErpData[30] * 128u + pErpData[31];
  int      lsd1      = (int) pErpData[32] * 128u + pErpData[33];
  int      lsd2      = (int) pErpData[34] * 128u + pErpData[35];

  if (lsd0 >= 8192)
    lsd0 = lsd0 - 16384;
  if (lsd1 >= 8192)
    lsd1 = lsd1 - 16384;
  if (lsd2 >= 8192)
    lsd2 = lsd2 - 16384;

  static int ehc0 = 1;
  static int ehc1 = 1;
  static int ehc2 = 1;
  static int ehc3 = 1;
  static int ehc4 = 1;
  static int ehc5 = 1;
  static int ehc6 = 1;
  static int ehc7 = 1;

  static int ihc0 = 1;
  static int ihc1 = 1;
  static int ihc2 = 1;
  static int ihc3 = 1;

  static int ali0 = 1;
  static int ali1 = 1;

  static int psu0 = 1;
  static int psu1 = 1;

  static int rotenc  = 0;
  static int buttons = 0;

  switch (getPacketNr(pErpData) & 0b111)
  {
    case 0:
      ehc0 = lsd0;
      ehc1 = lsd1;
      ehc2 = lsd2;
      break;
    case 1:
      ehc3 = lsd0;
      ehc4 = lsd1;
      ehc5 = lsd2;
      break;
    case 2:
      ehc6 = lsd0;
      ehc7 = lsd1;
      ihc0 = lsd2;
      break;
    case 3:
      ihc1 = lsd0;
      ihc2 = lsd1;
      ihc3 = lsd2;
      break;
    case 4:
      ali0 = lsd0;
      ali1 = lsd1;
      psu0 = lsd2;
      break;
    case 5:
      psu1 = lsd0;
      rotenc += lsd1;
      buttons = lsd2;
      break;
  }
#if SHOW_RAW
  // show all data
  cursorUp(2);
  printf("%+6.1lf ", angleErp0 == TWENTYONE_ONES ? -0.0001 : (double) angleErp0 * ERP_AngleMultiplier360());
  printf("%+6.1lf ", angleErp1 == TWENTYONE_ONES ? -0.0001 : (double) angleErp1 * ERP_AngleMultiplier360());
  printf("%+6.1lf ", angleErp2 == TWENTYONE_ONES ? -0.0001 : (double) angleErp2 * ERP_AngleMultiplier360());
  printf("%+6.1lf ", angleErp3 == TWENTYONE_ONES ? -0.0001 : (double) angleErp3 * ERP_AngleMultiplier360());
  printf("%+6.1lf ", angleErp4 == TWENTYONE_ONES ? -0.0001 : (double) angleErp4 * ERP_AngleMultiplier360());
  printf("%+6.1lf | ", angleErp5 == TWENTYONE_ONES ? -0.0001 : (double) angleErp5 * ERP_AngleMultiplier360());
  //printf("%+6.1lf ", angleErp6 == TWENTYONE_ONES ? -0.0001 : (double) angleErp6 * ERP_AngleMultiplier360());
  //printf("%+6.1lf | ", angleErp7 == TWENTYONE_ONES ? -0.0001 : (double) angleErp7 * ERP_AngleMultiplier360());

  double ref5VA = (psu1 - 1);

  printf("%4d(%4.2lfV) ", ehc0, (double) (ehc0 - 1) / ref5VA * 5.0);
  printf("%4d(%4.2lfV) ", ehc1, (double) (ehc1 - 1) / ref5VA * 5.0);
  printf("%4d(%4.2lfV) ", ehc2, (double) (ehc2 - 1) / ref5VA * 5.0);
  printf("%4d(%4.2lfV) ", ehc3, (double) (ehc3 - 1) / ref5VA * 5.0);
  printf("%4d(%4.2lfV) ", ehc4, (double) (ehc4 - 1) / ref5VA * 5.0);
  printf("%4d(%4.2lfV) ", ehc5, (double) (ehc5 - 1) / ref5VA * 5.0);
  printf("%4d(%4.2lfV) ", ehc6, (double) (ehc6 - 1) / ref5VA * 5.0);
  printf("%4d(%4.2lfV)\n", ehc7, (double) (ehc7 - 1) / ref5VA * 5.0);

  printf("%5d ", ihc0);
  printf("%5d ", ihc1);
  printf("%5d ", ihc2);
  printf("%5d | ", ihc3);
  printf("%5d ", ali0);
  printf("%5d ", ali1);
  printf("%5d(%5.1lfV) ", psu0, (double) (psu0 - 1) / 2046 / (1.72 / 3.3) * 19.0);
  printf("%5d(%5.2lfV)\n", psu1, (double) (psu1 - 1) / 2046 / (3.113 / 3.3) * 5.0);
  printf("S:%08X ", stat);
  printf("B:%02X ", buttons);
  printf("R:%+5d ", rotenc);
  printf("\n");
  cursorUp(1);

  return TRUE;
#endif

  int angle = angleErp0;
  if (angle == 0b111111111111111111111)
  {
    ++angleErrors;
    // cursorUp(1);
    // printf(">>>%u<<<\n\n", ++angleErrors);
  }
  else
  {
    if (angle & 0b100000000000000000000)
      angle |= 0b11111111111000000000000000000000;
    //printf("%8.2lf\n", (double) angle / ERP_SCALE_FACTOR);
    //return TRUE;
    //printf("%+9.3lf\n", angle * ERP_AngleMultiplier360()), cursorUp(1);
    static int   oldAngle;
    static char  green[]  = "\033[32;7;1m";
    static char  normal[] = "\033[39;0m";
    static char *color    = normal;

    int diff;
    int delta = ERP_getDynamicIncrement(quantizer, diff = 10 * ERP_GetAngleDifference(angle, oldAngle));
    oldAngle  = angle;

#if 01
    double        diffF         = angle * ERP_AngleMultiplier360();
    static double smoothedDiffF = 0.0;
    smoothedDiffF               = smoothedDiffF - (0.003 * (smoothedDiffF - diffF));
    printf("%+6.1lf\n", smoothedDiffF), cursorUp(1);
#endif

    static int sum = 10000;

    sum += delta;
    if (sum < 0)
      sum = 0;
    if (sum > 20000)
      sum = 20000;

    int update = 0;

    //printf("%8d\n", delta), cursorUp(1);

    static int touched = 1;

    if (touched != ERP_touched(quantizer))
    {
      update  = 1;
      touched = !touched;
      color   = touched ? green : normal;
    }
    if (delta)
      update = 1;

#if 0
    // determine smoothed angle
    static double oldAngle360;
    double        angle360         = angle * ERP_AngleMultiplier360();
    static double smoothedAngle360 = 0.0;
    if (smoothedAngle360 == 0.0)
      smoothedAngle360 = angle360;
    static int adjust = 0;

    if ((angle360 > +90.0 || angle360 < -90.0) && (oldAngle360 > +90.0 || oldAngle360 < -90.0))
    {  // danger zone
      adjust = 0;
      if (angle360 < 0 && oldAngle360 > 0)
      {  // jump cw to negative angle
        angle360 += 360.0;
      }
      else if (angle360 > 0 && oldAngle360 < 0)
      {  // jump ccw to positive angle
        angle360 -= 360.0;
      }
    }
    else
    {
      if (adjust == 0)
      {
        adjust = 1;
        if (smoothedAngle360 > +180.0)
          smoothedAngle360 -= 360.0;
        else if (smoothedAngle360 < -180.0)
          smoothedAngle360 += 360.0;
      }
    }
    oldAngle360 = angle360;

    printf("%s", normal);
    double smoothingCoeff;
    double smDelta = fabs(smoothedAngle360 - angle360);
    if (smDelta > 0.8)
      smoothingCoeff = 0.1;
    else if (smDelta > 0.6)
      smoothingCoeff = 0.03;
    else if (smDelta > 0.4)
      smoothingCoeff = 0.003;
    else
      smoothingCoeff = 0.001;

    smoothedAngle360 = smoothedAngle360 - (smoothingCoeff * (smoothedAngle360 - angle360));

    cursorUp(4);
    printf("%+8.1lf  %+9.2lf\n\n\n\n", angle360, smoothedAngle360);
#endif

    static const char subs[10] = "0123456789";
    if (update)
    {
      cursorUp(3);
      int cols = 100 * sum / 20000;
      printf("   ");
      for (int i = 0; i < cols; i++)
        printf("%c", i % 5 == 0 ? '.' : '_');
      printf("%c", subs[(10 * 100 * sum / 20000) % 10]);
      for (int i = cols + 1; i <= 100; i++)
        printf("%c", i % 5 == 0 ? '.' : ' ');
      printf("\n");

      for (int i = 0; i < cols; i++)
        printf(" ");

      printf("%s%+07.2lf%s ", color, (sum - 10000) / 100.0, normal);

      for (int i = cols + 3; i < 100; i++)
        printf(" ");

      printf("\n%s%+07.2lf%s\n", color, (sum - 10000) / 100., normal);
      fflush(stdout);
    }
  }
  printf("\n%6u packet errors, %6u angle errors \n", packetErrors, angleErrors);
  cursorUp(2);

  return TRUE;
}

static inline BOOL parseReceivedByte(uint8_t byte)
{
  static int step;

  static uint8_t  buf[PARSER_BUFFER_SIZE];
  static unsigned bufPos = 0;

  switch (step)
  {
    case 0:  // wait for sysex begin
      if (byte == 0xF0)
      {
        bufPos = 0;
        step   = 1;
        if (dump)
          printf("\n%2X ", byte);
      }
      return TRUE;
    case 1:              // sysex content
      if (byte == 0xF7)  // end of sysex ?
      {
        if (dump)
          printf("%02X\n", byte);
        step = 0;
        return examineContent(buf, bufPos);
      }
      if (dump)
        printf("%02X ", byte);
      if (byte & 0x80)
      {
        error("unexpected byte >= 0x80 within sysex!");
        return FALSE;
      }
      if (bufPos >= PARSER_BUFFER_SIZE)
      {
        error("unexpected buffer overrun (no sysex terminator found)!");
        return FALSE;
      }
      buf[bufPos++] = byte;
      break;
  }
  return TRUE;
}

static inline void doReceive(void)
{
  unsigned       npfds;
  struct pollfd *pfds;
  int            read;
  uint8_t        buf[RAW_RX_BUFFER_SIZE];
  int            err;
  unsigned short revents;

  snd_rawmidi_status_t *pStatus;
  snd_rawmidi_status_alloca(&pStatus);

  snd_rawmidi_params_t *pParams;
  snd_rawmidi_params_alloca(&pParams);

  snd_rawmidi_params_current(port, pParams);
  snd_rawmidi_params_set_buffer_size(port, pParams, sizeof buf);
  snd_rawmidi_params(port, pParams);
  snd_rawmidi_params_current(port, pParams);
  if (snd_rawmidi_params_get_buffer_size(pParams) != sizeof buf)
  {
    error("cannot set send buffer size of %d", sizeof buf);
    return;
  }

  if ((err = snd_rawmidi_nonblock(port, 1)) < 0)
  {
    error("cannot set non-blocking mode: %s", snd_strerror(err));
    return;
  }

  npfds = snd_rawmidi_poll_descriptors_count(port);
  pfds  = alloca(npfds * sizeof(struct pollfd));
  snd_rawmidi_poll_descriptors(port, pfds, npfds);
  signal(SIGINT, sigHandler);

  ERP_QuantizerInit_t erpInitData = {
    .sampleRate               = 2000,           // 2kHz
    .filterTimeConst          = 0.1,            // velocity lowpass
    .adaptiveFiltering        = 1,              // enabled
    .slidingWindowTime        = 0.1,            // increment averaging window
    .velocityStart            = 1,              // low speed
    .velocityStop             = 1000,           // high speed
    .incrementsPerDegreeStart = 0.01,           // fine resolution
    .incrementsPerDegreeStop  = 20000. / 180.,  // coarse resolution : 20000 increments over 180 deg of rotation
    .splitPointVelocity[0]    = 0.1,            // at this point within the transition region...
    .splitPointIncrement[0]   = 0.01,           // ...this is the output scale factor
    .splitPointVelocity[1]    = 0.3,            // at this point within the transition region...
    .splitPointIncrement[1]   = 0.1,            // ...this is the output scale factor
  };

  quantizer = ERP_InitQuantizer(erpInitData);
  if (!quantizer)
  {
    error("cannot initialize ERP quantizer");
    return;
  }

  printf("Receiving data from port: %s\n\n\n\n\n", pName);

  do
  {
    err = poll(pfds, npfds, 0);               // poll as fast as possible
    if (stop || (err < 0 && errno == EINTR))  // interrupted ?
      break;
    if (err < 0)
    {
      error("poll failed: %s", strerror(errno));
      break;
    }

    if ((err = snd_rawmidi_poll_descriptors_revents(port, pfds, npfds, &revents)) < 0)
    {
      error("cannot get poll events: %s", snd_strerror(errno));
      break;
    }
    if (revents & (POLLERR | POLLHUP))
      break;
    if (!(revents & POLLIN))
      continue;

    read = snd_rawmidi_read(port, buf, sizeof buf);
    if (read == -EAGAIN)
      continue;  // nothing read yet

    if (read < 0)
    {
      error("cannot read from port \"%s\": %s", pName, snd_strerror(read));
      break;
    }

    if ((err = snd_rawmidi_status(port, pStatus)))
    {
      error("cannot get status: %s", snd_strerror(errno));
      break;
    }

    for (int i = 0; i < read; ++i)
    {
      if (!parseReceivedByte(buf[i]))  // parser error
      {
        for (int j = 0; j < read; j++)
        {
          if (j == i)
            printf(">>%02X<< ", buf[j]);  // mark the offending byte
          else
            printf("%02X ", buf[j]);
        }
        stop = TRUE;
      }
    }
    if ((err = snd_rawmidi_status_get_xruns(pStatus)))
    {
      error("rawmidi receive buffer overrun: %d", err);
      break;
    }
  } while (TRUE);

  ERP_ExitQuantizer(quantizer);

  printf("\n");
  fflush(stdout);
}

//
// ------------------------------------------
//
int main(int argc, char const *const argv[])
{
  getCmdLineParams(argc, argv);
  openPort();
  if (send)
    doSend();
  else
    doReceive();
  closePort();
  return 0;
}
