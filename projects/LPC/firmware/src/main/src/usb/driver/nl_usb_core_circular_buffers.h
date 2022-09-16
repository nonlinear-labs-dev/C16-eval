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

#ifdef INCLUDED_FROM_USB_CORE

static inline int within(uint32_t const p, uint32_t const* const start, unsigned const length)
{
  return (p >= (uint32_t) start) && (p < (uint32_t) start + length);
}

// detect buffer type by comparing addresses of our circular buffers above
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
