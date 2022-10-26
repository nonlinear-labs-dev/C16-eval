#pragma once

#include <inttypes.h>
#include "io/pins.h"
#include "tasks/mtask.h"

namespace Task
{

  class Encoder : public Task::Task
  {
   private:
    StateMonitor::StateMonitor& m_stateMonitor;
    unsigned                    read_step { 0 };       // step-chain variable
    uint8_t                     old_pattern { 0xFF };  // force initial difference
    int                         direction { 0 };       // -1 == left(inc), +1 == right(dec), 0 == no change
    int                         enc_delta { 0 };

   public:
    constexpr Encoder(uint32_t const delay, uint32_t const period, StateMonitor::StateMonitor& stateMonitor)
        : Task(delay, period)
        , m_stateMonitor(stateMonitor) {};

    inline void body(void)
    {
      uint8_t pattern;  // bit 0 == A, bit 1 == B
      int     retVal = 0;

      pattern = 0;
      if (pinENC_A)
        pattern |= 0b01;
      if (pinENC_B)
        pattern |= 0b10;

      if (pattern != old_pattern)  // state change ?
      {
        retVal = +1;
        if ((pattern ^ old_pattern) == 0b11)
        {  // both bits have flipped, we missed a transition --> reset step-chain
          read_step = 0;
          retVal    = -1;
        }
        old_pattern = pattern;

        switch (read_step)
        {
          case 0:  // wait for idle state (until A == B)
            if (pattern == 0b00)
              read_step = 10;  // start from both low
            else if (pattern == 0b11)
              read_step = 20;  // start from both high
            break;

          case 10:                // wait for first transition, starting both low
            if (pattern == 0b01)  // A went high --> increment
            {
              direction = +1;
              read_step = 11;  // wait for next transition
            }
            else if (pattern == 0b10)  // B went high --> decrement
            {
              direction = -1;
              read_step = 11;  // wait for next transition,
            }
            break;

          case 11:                // wait for second transition, either both going high or low
            if (pattern == 0b11)  // both went high --> trigger
            {
              enc_delta += direction;
              direction = 0;
              read_step = 20;  // step chain starts again with both high
            }
            else if (pattern == 0b00)  // both went low again --> operation incomplete, reset
            {
              direction = 0;
              read_step = 10;  // reset step chain
              retVal    = -1;
            }
            break;

          case 20:                // wait for first transition, starting both high
            if (pattern == 0b10)  // A went low --> increment
            {
              direction = +1;
              read_step = 21;  // wait for next transition
            }
            else if (pattern == 0b01)  // B went low --> decrement
            {
              direction = -1;
              read_step = 21;  // wait for next transition,
            }
            break;

          case 21:                // wait for second transition, either both going low or high
            if (pattern == 0b00)  // both went low --> trigger
            {
              enc_delta += direction;
              direction = 0;
              read_step = 10;  // step chain starts again with both low
            }
            else if (pattern == 0b11)  // both went high again --> operation incomplete, reset
            {
              direction = 0;
              read_step = 20;  // reset step chain
              retVal    = -1;
            }
            break;
        }
      }
      if (retVal == -1)
        m_stateMonitor.event(StateMonitor::ERROR_ENCODER_OVERRUN);
    };

    inline int getAndClearEncoderDelta(void)
    {
      int tmp_delta = enc_delta;

      if (enc_delta > +16383)
        tmp_delta = +16383;
      else if (enc_delta < -16384)
        tmp_delta = -16384;

      enc_delta = 0;

      return tmp_delta;
    };

  };  // class Encoder

}  // namespace
