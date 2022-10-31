#pragma once

#include <inttypes.h>
#include "io/pins.h"
#include "tasks/mtask.h"

namespace Task
{

  class Encoder
  {
   private:
    enum class States
    {
      WaitForIdle_SameState,
      WaitForFirstTransition_fromBothLow,
      WaitForSecondTransition_fromBothLow,
      WaitForFirstTransition_fromBothHigh,
      WaitForSecondTransition_fromBothHigh,
    };

    enum States m_state
    {
      States::WaitForIdle_SameState
    };                              // step-chain state variable
    uint8_t m_oldPattern { 0xFF };  // force initial difference
    int     m_direction { 0 };      // -1 == left(inc), +1 == right(dec), 0 == no change
    int     m_stepsAccumulator { 0 };

   public:
    inline void run(void)
    {
      uint8_t pattern = uint8_t(pinENC_B << 1) | uint8_t(pinENC_A);

      if (pattern == m_oldPattern)
        return;

      if ((pattern ^ m_oldPattern) == 0b11)  // both bits have flipped, we missed a transition --> full reset step-chain
        m_state = States::WaitForIdle_SameState;

      m_oldPattern = pattern;

      switch (m_state)
      {
        case States::WaitForIdle_SameState:
          if (pattern == 0b00)
            m_state = States::WaitForFirstTransition_fromBothLow;
          else if (pattern == 0b11)
            m_state = States::WaitForFirstTransition_fromBothHigh;
          return;

        case States::WaitForFirstTransition_fromBothLow:
          if (pattern == 0b01)  // A went high --> increment
            m_direction = +1, m_state = States::WaitForSecondTransition_fromBothLow;
          else if (pattern == 0b10)  // B went high --> decrement
            m_direction = -1, m_state = States::WaitForSecondTransition_fromBothLow;
          return;

        case States::WaitForSecondTransition_fromBothLow:
          if (pattern == 0b11)  // both are high now --> trigger event
            m_stepsAccumulator += m_direction, m_state = States::WaitForFirstTransition_fromBothHigh;
          else if (pattern == 0b00)  // both went low again --> operation incomplete (bouncing), restart
            m_state = States::WaitForFirstTransition_fromBothLow;
          return;

        case States::WaitForFirstTransition_fromBothHigh:
          if (pattern == 0b10)  // A went low --> increment
            m_direction = +1, m_state = States::WaitForSecondTransition_fromBothHigh;
          else if (pattern == 0b01)  // B went low --> decrement
            m_direction = -1, m_state = States::WaitForSecondTransition_fromBothHigh;
          return;

        case States::WaitForSecondTransition_fromBothHigh:
          if (pattern == 0b00)  // both are low now --> trigger event
            m_stepsAccumulator += m_direction, m_state = States::WaitForFirstTransition_fromBothLow;
          else if (pattern == 0b11)  // both went high again --> operation incomplete (bouncing), restart
            m_state = States::WaitForFirstTransition_fromBothHigh;
          break;
      }  // switch
    };

    inline int getAndClearEncoderDelta(void)
    {
      asm volatile("cpsid i");  // mutex

      int stepsAccumulator = m_stepsAccumulator;
      if (m_stepsAccumulator > +16383)
        stepsAccumulator = +16383;
      else if (m_stepsAccumulator < -16384)
        stepsAccumulator = -16384;
      m_stepsAccumulator = 0;

      asm volatile("cpsie i");

      return stepsAccumulator;
    };

  };  // class Encoder

}  // namespace
