#ifndef PWM_LED_H
#define PWM_LED_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

void setup_pwm();
void main_pwm_led(uint16_t sw_value);

#endif