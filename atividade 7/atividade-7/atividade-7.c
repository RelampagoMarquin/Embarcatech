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
volatile int sw_pressed = 1; // Variável que verifica o botão SW

// variaveis do menu
const char *menu_options[] = {"BATIMENTOS", "GLICEMIA"};
int menu_index = 0;
int total_options = 3;

// variaveis de monitoramento
volatile int frequencia_cardiaca = 80;
volatile int glicemia = 100;

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

// Navegação do menu com joystick
void navigate_menu()
{
    uint16_t vrx_value, vry_value;
    joystick_read_axis(&vrx_value, &vry_value);

    if (vry_value > 3000)
    {
        menu_index = (menu_index - 1 + total_options) % total_options; // Cima
    }
    else if (vry_value < 1000)
    {
        menu_index = (menu_index + 1) % total_options; // Baixo
    }

    display_menu();
    sleep_ms(300);
}

// Função de ajuste de frequencia cardiaca
int ajustar_batimento(int frequencia_cardiaca)
{
    uint16_t vrx_value, vry_value;
    joystick_read_axis(&vrx_value, &vry_value);

    // Ajusta a frequência cardíaca baseado no valor do analógico
    if (vrx_value > 3000)
    {
        frequencia_cardiaca += 1;
    }
    else if (vrx_value < 1000)
    {
        frequencia_cardiaca -= 1;
    }

    // Limita a faixa da frequência cardíaca
    if (frequencia_cardiaca < 40)
        frequencia_cardiaca = 40;
    if (frequencia_cardiaca > 180)
        frequencia_cardiaca = 180;

    // Exibe no display
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "- %d +", frequencia_cardiaca);
    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    ssd1306_draw_string(ssd, 5, 30, buffer);
    render_on_display(ssd, &frame_area);

    return frequencia_cardiaca;
}

// Função de ajuste de glicemia
int ajustar_glicemia(int glicemia)
{
    uint16_t vrx_value, vry_value;
    joystick_read_axis(&vrx_value, &vry_value);

    // Ajusta a frequência cardíaca baseado no valor do analógico
    if (vrx_value > 3000)
    {
        glicemia += 1;
    }
    else if (vrx_value < 1000)
    {
        glicemia -= 1;
    }

    // Limita a faixa da frequência cardíaca
    if (glicemia < 50)
        glicemia = 50;
    if (glicemia > 160)
        glicemia = 160;

    // Exibe no display
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "- %d +", glicemia);
    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    ssd1306_draw_string(ssd, 5, 30, buffer);
    render_on_display(ssd, &frame_area);

    return glicemia;
}

void execute_selection()
{
    switch (menu_index)
    {
    case 0:
        // ajustar batimento
        break;
    case 1:
        // ajustar glicemia
        break;
    }
}

void espera_com_leitura(int timeMS)
{
    for (int i = 0; i < timeMS; i += 100)
    {
        if (!gpio_get(SW)) // Verifica se o botão foi pressionado
        {
            sw_pressed = 1;
        }
        sleep_ms(100);
    }
}

void chama_menu()
{
    // verifica se foi ativado o menu
    if (sw_pressed == 1)
    {
        navigate_menu();
        if (!gpio_get(SW)) // Verifica se o botão foi pressionada
        {
            execute_selection();
            sleep_ms(300);
            display_menu();
        }
        sw_pressed = 0;
    }
}

void apagar_led(int intervalo)
{
    espera_com_leitura(intervalo);
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
    espera_com_leitura(intervalo);
}

void monitoramento(int glicemia, int frequencia_cardiaca)
{
    int intervalo = 60000 / frequencia_cardiaca / 2; // Tempo de um ciclo (ms) / 2

    // Vermelho - nivel de glicemia critico
    if (glicemia > 140 || glicemia < 70)
    {
        gpio_put(LED_R_PIN, 1);
        gpio_put(LED_G_PIN, 0);
        gpio_put(LED_B_PIN, 0);
    }
    else if ((glicemia >= 70 && glicemia <= 75) || (glicemia >= 135 && glicemia <= 140))
    {
        // Amarelo - nivel de glicemia em alerta
        gpio_put(LED_R_PIN, 1);
        gpio_put(LED_G_PIN, 1);
        gpio_put(LED_B_PIN, 0);
    }
    else
    {
        // Verde - nivel normal de glicemia
        gpio_put(LED_R_PIN, 0);
        gpio_put(LED_G_PIN, 1);
        gpio_put(LED_B_PIN, 0);
    }

    apagar_led(intervalo); // apaga o led
}

void exibe_situcao(int freq_cardiaca, int glice)
{
    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    char status_bpm[16];
    char status_glc[16];
    snprintf(status_bpm, sizeof(status_bpm), "BPM:%d", frequencia_cardiaca);
    snprintf(status_glc, sizeof(status_glc), "Glc:%d", glicemia);
    ssd1306_draw_string(ssd, 5, 20, status_bpm);
    ssd1306_draw_string(ssd, 5, 40, status_glc);
    render_on_display(ssd, &frame_area);
}

void led_setup()
{
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);
}

int main()
{
    setup_joystick();
    led_setup();
    init_oled();
    display_menu();

    while (true)
    {
        if (sw_pressed == 1)
        {
            sleep_ms(300);
            chama_menu();
            sw_pressed = 0;
        }
        // rotina normal
        exibe_situcao(frequencia_cardiaca, glicemia);
        monitoramento(glicemia, frequencia_cardiaca);
    }
    sleep_ms(300);
}