
#ifndef BTS7960_h
#define BTS7960_h

#include <Arduino.h>

class BTS7960
{
public:
    BTS7960(int L_EN, int R_EN, int PWM);
    void SetSpeed(int pwm);
    void Stop();

private:
    int _L_EN;
    int _R_EN;
    int _PWM;
};
#endif