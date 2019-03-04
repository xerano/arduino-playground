#include "PID.h"

//#define DEBUG

PID::PID(float kp, float kd, float ki){
  this->kp = kp;
  this->kd = kd;
  this->ki = ki;
  
}

int PID::calculate(int setpoint, int current){
  int error = setpoint - current;

  p = kp * error;
  i += (ki * error);

  time_prev = time;
  time = millis();
  elapsed = (time - time_prev) / 1000.0;

  d = kd * ((error - previous_error) / elapsed);
  previous_error = error;
  
  int val = p + i;
  #ifdef DEBUG
  Serial.print("PID: p:");
  Serial.print(p);
  Serial.print(" i:");
  Serial.print(i);
  Serial.print(" d:");
  Serial.print(d);
  Serial.print(" val:");
  Serial.println(val);
  #endif

  if(val < 0){
    i = 0;
    val = 0;
  }
  if(val > 255){
    i = 0;
    val = 255;
  }
  
  return val;
}
