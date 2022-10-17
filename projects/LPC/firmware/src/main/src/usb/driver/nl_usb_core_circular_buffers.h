//
// LPC43xx USB has the nice feature of arbitrary 4k "page register" for transfers,
// allowing for automatic circular buffer behavior.
//
#pragma once

#include <stdint.h>

enum USB_BufferType
{
  USB_NON_CIRCULAR = 0,
  USB_CIRCULAR_4k  = 4096u,
  USB_CIRCULAR_8k  = 8192u,
  USB_CIRCULAR_16k = 16384u,
};

// 40kBytes for various circular buffers for USB
typedef struct
{
  uint32_t buffer_16k_0[16384u / sizeof(uint32_t)];
  uint32_t buffer_8k_0[8192u / sizeof(uint32_t)];
  uint32_t buffer_8k_1[8192u / sizeof(uint32_t)];
  uint32_t buffer_4k_0[4096u / sizeof(uint32_t)];
  uint32_t buffer_4k_1[4096u / sizeof(uint32_t)];
} USB_circularBuffers_t;

extern USB_circularBuffers_t USB_circular;

// link buffers to applications
#define USB_BUFFER_FOR_SENSOR_DATA USB_circular.buffer_16k_0
#define USB_BUFFER_BRIDGE_TO_HOST  USB_circular.buffer_8k_0
#define USB_BUFFER_HOST_TO_BRIDGE  USB_circular.buffer_8k_1

#ifdef INCLUDED_FROM_USB_CORE

static inline int within(uint32_t const p, uint32_t const* const start, unsigned const length)
{
  return (p >= (uint32_t) start) && (p < (uint32_t) start + length);
}

// detect buffer type by comparing to addresses of our circular buffers above
static inline enum USB_BufferType NL_USB_getBbufferStategy(uint32_t const ptrBuff)
{
  if (within(ptrBuff, USB_circular.buffer_16k_0, 16384))
    return USB_CIRCULAR_16k;
  else if (within(ptrBuff, USB_circular.buffer_8k_0, 8192) || within(ptrBuff, USB_circular.buffer_8k_1, 8192))
    return USB_CIRCULAR_8k;
  else if (within(ptrBuff, USB_circular.buffer_4k_0, 4096) || within(ptrBuff, USB_circular.buffer_4k_1, 4096))
    return USB_CIRCULAR_4k;
  return USB_NON_CIRCULAR;
}

#endif
