#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

// Definição dos pinos do joystick e do botão
#define VRX 26
#define VRY 27
#define SW 22

// Definição dos pinos do LED
#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12

// Definição do I2C para o display OLED
#define I2C_SDA 14
#define I2C_SCL 15

const char *menu_options[] = {"Sinal Aberto", "Marcha Imperial", "Sinal Fechado"};
int menu_index = 0;
int total_options = 3;

void setup_joystick()
{
    adc_init();
    adc_gpio_init(VRX);
    adc_gpio_init(VRY);
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);
}

void init_oled()
{
    stdio_init_all();
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();
}

void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value)
{
    adc_select_input(0);
    sleep_us(2);
    *vry_value = adc_read();
    adc_select_input(1);
    sleep_us(2);
    *vrx_value = adc_read();
}

void display_message(const char *message[])
{
    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    for (int i = 0; message[i] != NULL; i++)
    {
        char temp_buffer[22];
        strncpy(temp_buffer, message[i], sizeof(temp_buffer) - 1);
        temp_buffer[sizeof(temp_buffer) - 1] = '\0'; // Garante terminação correta
        ssd1306_draw_string(ssd, 5, i * 10, temp_buffer);
    }
    render_on_display(ssd, &frame_area);
}

void display_menu()
{
    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    for (int i = 0; i < total_options; i++)
    {
        char buffer[22] = {0};
        snprintf(buffer, sizeof(buffer), "%s%s", (i == menu_index) ? " " : "", menu_options[i]);
        ssd1306_draw_string(ssd, 5, i * 10, buffer);
    }
    render_on_display(ssd, &frame_area);
}

void navigate_menu()
{
    uint16_t vrx_value, vry_value;
    joystick_read_axis(&vrx_value, &vry_value);
    if (vry_value > 3000)
    {
        menu_index = (menu_index - 1 + total_options) % total_options;
        display_menu();
        sleep_ms(200);
    }
    else if (vry_value < 1000)
    {
        menu_index = (menu_index + 1) % total_options;
        display_menu();
        sleep_ms(200);
    }
}

void SinalAberto()
{
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
    const char *message[] = {"SINAL ABERTO", "ATRAVESSAR COM CUIDADO", NULL};
    display_message(message);
}

void SinalAtencao()
{
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
    const char *message[] = {"SINAL DE ATENCAO", "PREPARE-SE", NULL};
    display_message(message);
}

void SinalFechado()
{
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
    const char *message[] = {"SINAL FECHADO", "AGUARDE", NULL};
    display_message(message);
}

// mudar esse trecho para um arquivo externo
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

void beep(uint pin, float duration_ms)
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

void imperialMarch()
{
#define ALERTA 21
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

    gpio_init(ALERTA);
    gpio_set_dir(ALERTA, GPIO_OUT);

    for (int i = 0; i <= 33; i++)
    {
        if (i > 1 && i % 8 == 0)
        {
            sleep_ms(10);
        }
        pwm_init_buzzer(ALERTA, star_wars_notes[i]);
        beep(ALERTA, note_duration[i]);
    }
    sleep_ms(1000);
}

void execute_selection()
{
    switch (menu_index)
    {
    case 0:
        SinalAberto();
        break;
    case 1:
        imperialMarch();
        break;
    case 2:
        SinalFechado();
        break;
    }
}

void setup_leds()
{
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_put(LED_R_PIN, 0);

    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_put(LED_G_PIN, 0);

    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);
    gpio_put(LED_B_PIN, 0);
}

int main()
{
    setup_joystick();
    setup_leds();
    init_oled();
    display_menu();

    while (true)
    {
        navigate_menu();
        if (!gpio_get(SW))
        {
            execute_selection();
            sleep_ms(1000);
            display_menu();
        }
    }
}