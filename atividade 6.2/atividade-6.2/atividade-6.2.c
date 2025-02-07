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
#include "hardware/watchdog.h"
#include "buzzer_pwm/imperial-march.h"
#include "pwm_led/pwm-led.h"
#include "joystick_led/joystick-led.h"

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

// buzzer
#define BUZZER_PIN 21

// variaveis do menu
const char *menu_options[] = {"Joystick Led", "Buzzer PWM", "Led PWM"};
int menu_index = 0;
int total_options = 3;


// inicializa o joystick
void setup_joystick()
{
    adc_init();
    adc_gpio_init(VRX);
    adc_gpio_init(VRY);
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);
}

// função de inicializar o OLED
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

// função de leitura do joystick
void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value)
{
    adc_select_input(0);
    sleep_us(2);
    *vry_value = adc_read();

    adc_select_input(1);
    sleep_us(2);
    *vrx_value = adc_read();
}

// Função de exibir o menu
void display_menu()
{
    
    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    ssd1306_draw_string(ssd, 5, 0, "    MENU    ");
    for (int i = 0; i < total_options; i++)
    {
        char buffer[16] = {0};
        snprintf(buffer, sizeof(buffer), "%s%s", (i == menu_index) ? "x " : "", menu_options[i]);
        ssd1306_draw_string(ssd, 5, (i + 1) * 18, buffer);
    }
    render_on_display(ssd, &frame_area);
}

// Sub Menu para exibir após escolher uma função
void sub_menu()
{
    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    char buffer[22] = {0};
    snprintf(buffer, sizeof(buffer), "X sair");
    ssd1306_draw_string(ssd, 5, 10, buffer);

    if(menu_index == 1){
        ssd1306_draw_string(ssd, 5, 24, "Aguarde a pausa");
        ssd1306_draw_string(ssd, 5, 36, "da musica para");
        ssd1306_draw_string(ssd, 5, 44, "apertar e sair");
    }
    render_on_display(ssd, &frame_area);
}

// Função de navegação do Menu
void navigate_menu()
{
    uint16_t vrx_value, vry_value;              // declara as varaveis do joystick
    joystick_read_axis(&vrx_value, &vry_value); // função de leitura do joystick
    if (vry_value > 3000)
    {
        menu_index = (menu_index - 1 + total_options) % total_options; // seleciona opção do menu para cima
    }
    else if (vry_value < 1000)
    {
        menu_index = (menu_index + 1) % total_options; // seleciona opção do menu para baixo
    }
    display_menu(); // exbibe o menu com atualizado com a opção
    sleep_ms(300);
}


void execute_selection()
{
    sub_menu();
    switch (menu_index)
    {
    case 0:
        // chama a biblioteca do Joystick_led
        uint16_t vrx_value, vry_value;
        joystick_led(LED_B_PIN, LED_R_PIN, vrx_value, vry_value, SW);
        break;
    case 1:
        // chama a biblioteca do BUZZER_PWM
        pwm_init_buzzer(BUZZER_PIN);
        play_star_wars(BUZZER_PIN, SW);
        break;
    case 2:
        // cativa a biblioteca do PWM_LED
        main_pwm_led(SW);
        break;
    }
}

int main()
{
    setup_joystick();
    init_oled();
    display_menu();

    while (true)
    {
        navigate_menu();
        if (!gpio_get(SW)) // Verifica se o botão foi pressionada
        {
            execute_selection();
            sleep_ms(300);
            display_menu();
        }
    }
}