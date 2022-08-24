#pragma once

#include "io/pins.h"
#include "tasks/mtask.h"

namespace Task
{

  class LedErrorWarning : public Task::Task
  {
    using Task::Task;

   private:
    int m_errorTimer;
    int m_warningTimer;
    int m_errorTimerFlicker;
    int m_warningTimerFlicker;

   public:
    LedErrorWarning(uint32_t delay, uint32_t period)
        : Task(delay, period)
        , m_errorTimer(0)
        , m_warningTimer(0)
        , m_errorTimerFlicker(0)
        , m_warningTimerFlicker(0) {};

    inline void Error_TimedOn(int time)
    {
      if (time == 0)
        return;
      m_errorTimerFlicker = m_errorTimer != 0 && time > 0;
      if (time < 0)
        time = -time;
      if (time >= m_errorTimer)
        m_errorTimer = time;
      LED_ERROR = !m_errorTimerFlicker;
    };

    void Warning_TimedOn(int time)
    {
      if (time == 0)
        return;
      m_warningTimerFlicker = m_warningTimer != 0 && time > 0;
      if (time < 0)
        time = -time;
      if (time >= m_warningTimer)
        m_warningTimer = time;
      LED_WARNING = !m_warningTimerFlicker;
    };

    inline virtual void body(void)
    {
      if (m_errorTimerFlicker)
      {
        if (!--m_errorTimerFlicker)
          LED_ERROR = 1;
      }
      else if (m_errorTimer)
      {
        if (!--m_errorTimer)
          LED_ERROR = 0;
      }

      if (m_warningTimerFlicker)
      {
        if (!--m_warningTimerFlicker)
          LED_WARNING = 1;
      }
      else if (m_warningTimer)
      {
        if (!--m_warningTimer)
          LED_WARNING = 0;
      }
    };
  };

}
