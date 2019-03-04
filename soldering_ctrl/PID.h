#ifndef PID_H
#define PID_H

#include "Arduino.h"

class PID {
  public:
    PID(float kp, float kd, float ki);
    int calculate(int setpoint, int current);
  private:
    float kp, kd, ki, elapsed;
    int p = 0, i = 0, d = 0;
    int previous_error;
    unsigned long time = 0, time_prev = 0;
};

#endif
