#pragma once

#include <stdint.h>

typedef enum
{
  ERP_TRANS_CONVEX,
  ERP_TRANS_LINEAR,
  ERP_TRANS_CONCAVE
} decelerationCurve_t;

typedef struct
{
  uint16_t sampleRate;         // Hz, [500...5000]
  double   filterTimeConst;    // velocity lowpass filter time constant (in seconds)
  int      adaptiveFiltering;  // flag for dynamic time constant, tc ~ 1/velocity (at low velocities)
  double   slidingWindowTime;  // increments sliding window average (in seconds)

  // linear interpolation with four points to allow for some curvature
  // velocity points must be monotonically rising !
  uint16_t velocityStart;             // start velocity for deceleration (>= 0, <= 3000 deg/sec), moderate movements give values ~500deg/s
  uint16_t velocityStop;              // stop velocity for deceleration (>= start, <= 3000 deg/sec)
  double   incrementsPerDegreeStart;  // start (fine) resolution in increments per degrees units, <= 10 incr/deg
  double   incrementsPerDegreeStop;   // stop (coarse) resolution in increments per degrees units, normally > start;
  double   splitPointVelocity[2];     // split point ]0...1[  0.5 -> center
  double   splitPointIncrement[2];    // split point ]0...1[  0.5 -> center
} ERP_QuantizerInit_t;

struct ERP_Quantizer_t;

void *ERP_InitQuantizer(const ERP_QuantizerInit_t initData);
int   ERP_ExitQuantizer(void *const quantizer);
int   ERP_getDynamicIncrement(void *const quantizer, int const increment);
int   ERP_touched(void *const quantizer);
