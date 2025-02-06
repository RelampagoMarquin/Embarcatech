#ifndef JOYSTICK_PWM_H
#define JOYSTICK_PWM_H

#include <stdio.h>
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"


// Declaração das funções
void setup_joystick();
void setup_pwm_led(uint led, uint *slice, uint16_t level);
void setup(int LED_B, int LED_R);
void joystick_read_axis_joystick_led(uint16_t *vrx_value, uint16_t *vry_value);
void joystick_led(int LED_B, int LED_R, uint16_t vrx_value, uint16_t vry_value, uint16_t sw_value);

#endif // JOYSTICK_PWM_H
