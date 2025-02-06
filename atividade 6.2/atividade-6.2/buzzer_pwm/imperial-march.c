#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

// Configuração do pino do buzzer
#define BUZZER_PIN 21

volatile bool sw_pressed = false; // Variável que verifica o botão SW

// Notas musicais para a música tema de Star Wars
int star_wars_notes[] = {
    220, 220, 220, 174, 261, 220, 174, 261, 220,
    329, 329, 329, 349, 261, 207, 174, 261, 220,
    440, 220, 220, 440, 415, 392, 370, 329, 349,
    233, 311, 293, 277, 261, 247, 261

};

int note_duration[] = {
    500, 500, 500, 333, 166, 500, 333, 166, 500,
    500, 500, 500, 333, 166, 500, 333, 166, 500,
    500, 333, 166, 500, 333, 166, 100, 100, 100,
    166, 500, 333, 166, 100, 100, 100};

// Inicializa o PWM no pino do buzzer
void pwm_init_buzzer(uint pin, int FREQ_ALERTA)
{
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com frequência desejada
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (FREQ_ALERTA * 28672)); // Divisor de clock
    pwm_init(slice_num, &config, true);

    // Iniciar o PWM no nível baixo
    pwm_set_gpio_level(pin, 0);
}

// Toca uma nota com a frequência e duração especificadas
void play_tone(uint pin, float duration_ms)
{
    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o duty cycle para 50% (ativo)
    pwm_set_gpio_level(pin, 2048);

    // Temporização
    sleep_ms(duration_ms);

    // Desativar o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);

    // Pausa entre os beeps
    sleep_ms(100); // Pausa de 100ms
}

// verifica os eventos no botão do analogico
void sw_callback(uint gpio, uint32_t events) {
    sw_pressed = true;  // Marca que o botão foi pressionado
}

// verifica interação do botão
void setup_sw_interrupt(uint16_t sw_value) {
    gpio_set_irq_enabled_with_callback(sw_value, GPIO_IRQ_EDGE_FALL, true, &sw_callback);
}

// Função principal para tocar a música
void play_star_wars(uint pin, uint16_t sw_value)
{
    int again = true;
    setup_sw_interrupt(sw_value); // Ativa a interrupção no botão SW
    while (again)
    {
        for (int i = 0; i < sizeof(star_wars_notes) / sizeof(star_wars_notes[0]); i++)
        {
            if (i > 1 && i % 8 == 0)
            {
                sleep_ms(20);
                if (sw_pressed)
                {
                    again = false;
                }
            }
            pwm_init_buzzer(pin, star_wars_notes[i]);
            play_tone(pin, note_duration[i]);
        }
        sleep_ms(2000);
        if (sw_pressed)
        {
            again = false;
        }
    }
}