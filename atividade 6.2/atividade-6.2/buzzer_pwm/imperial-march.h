#ifndef IMPERIAL_MARCH_H
#define IMPERIAL_MARCH_H

#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/clocks.h>

#define BUZZER_PIN 21

void pwm_init_buzzer(uint pin);
void play_tone(uint pin, uint frequency, uint duration_ms);
void play_star_wars(uint pin, uint16_t sw_value);

#endif // IMPERIAL_MARCH_H