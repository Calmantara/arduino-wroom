#include <Arduino.h>
#include "BTS7960.h"

BTS7960::BTS7960(int L_EN, int R_EN, int PWM)
{
    _PWM = PWM;
    _L_EN = L_EN;
    _R_EN = R_EN;
    pinMode(PWM, OUTPUT);
    pinMode(_L_EN, OUTPUT);
    pinMode(_R_EN, OUTPUT);
}

void BTS7960::SetSpeed(int pwm)
{
    if (pwm > 0)
    {
        //CW rotation
        digitalWrite(_R_EN, HIGH);
        digitalWrite(_L_EN, LOW);
    }
    else
    {
        //CCW rotation
        digitalWrite(_R_EN, LOW);
        digitalWrite(_L_EN, HIGH);
    }

    analogWrite(_PWM, abs(pwm));
}

void BTS7960::Stop()
{
    digitalWrite(_R_EN, LOW);
    digitalWrite(_L_EN, LOW);
}
