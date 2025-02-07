#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pwm-led.h"

const uint LED = 12;            // Pino do LED conectado
const uint16_t PERIOD = 2000;   // Período do PWM (valor máximo do contador)
const float DIVIDER_PWM = 16.0; // Divisor fracional do clock para o PWM
const uint16_t LED_STEP = 100;  // Passo de incremento/decremento para o duty cycle do LED
uint16_t led_level = 100;       // Nível inicial do PWM (duty cycle)

volatile bool is_sw_pressed = false;
void setup_pwm()
{
    uint slice;
    gpio_set_function(LED, GPIO_FUNC_PWM); // Configura o pino do LED para função PWM
    slice = pwm_gpio_to_slice_num(LED);    // Obtém o slice do PWM associado ao pino do LED
    pwm_set_clkdiv(slice, DIVIDER_PWM);    // Define o divisor de clock do PWM
    pwm_set_wrap(slice, PERIOD);           // Configura o valor máximo do contador (período do PWM)
    pwm_set_gpio_level(LED, led_level);    // Define o nível inicial do PWM para o pino do LED
    pwm_set_enabled(slice, true);          // Habilita o PWM no slice correspondente
}

// verifica os eventos no botão do analogico
void sw_callback_pwm(uint gpio, uint32_t events) {
    is_sw_pressed = true;  // Marca que o botão foi pressionado
}

void main_pwm_led(uint16_t sw_value)
{
    sleep_ms(50); // para evitar o bouce
    uint up_down = 1; // Variável para controlar se o nível do LED aumenta ou diminui
    stdio_init_all(); // Inicializa o sistema padrão de I/O
    setup_pwm();      // Configura o PWM
    // ativa o monitor de eventos para o botao do analogico
    is_sw_pressed = false; // garantir que o estado do botão começa como false
    gpio_set_irq_enabled_with_callback(sw_value, GPIO_IRQ_EDGE_FALL, true, &sw_callback_pwm);
    int again = true; // variavel de controle
    while (again)
    {
        pwm_set_gpio_level(LED, led_level); // Define o nível atual do PWM (duty cycle)
        if (up_down)
        {
            led_level += LED_STEP; // Incrementa o nível do LED
            if (led_level >= PERIOD)
                up_down = 0; // Muda direção para diminuir quando atingir o período máximo
        }
        else
        {
            led_level -= LED_STEP; // Decrementa o nível do LED
            if (led_level <= LED_STEP)
                up_down = 1; // Muda direção para aumentar quando atingir o mínimo
        }
        sleep_ms(1000);                     // Atraso de 1 segundo
        if (is_sw_pressed)
        {
            pwm_set_gpio_level(LED, 0); // reseta o led para o 0
            again = false; // acaba o loop e sai da função
        }
    }
}