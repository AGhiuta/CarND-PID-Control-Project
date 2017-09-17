#include <iostream>
#include <stdlib.h>
#include "PID.h"

using namespace std;

PID::PID() {}

PID::~PID() {}

void PID::Init(double Kp, double Ki, double Kd) {
  this->Kp = Kp;
  this->Ki = Ki;
  this->Kd = Kd;
  this->p_error = 0;
  this->i_error = 0;
  this->d_error = 0;
}

void PID::UpdateError(double cte, double dt) {
  d_error = (cte - p_error) / dt;
  p_error = cte;
  i_error += cte * dt;
}

double PID::TotalError() {
  return Kp * p_error + Ki * i_error + Kd * d_error;
}