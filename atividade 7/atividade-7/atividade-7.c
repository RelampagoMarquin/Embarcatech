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
#include "wifi_parametros.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"

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
volatile bool sw_pressed = false; // Variável que verifica o botão SW

// variaveis do menu
const char *menu_options[] = {"BATIMENTOS", "GLICEMIA"};
int menu_index = 0;
int total_options = 2;
volatile int no_menu = 1;

// variaveis de monitoramento
volatile int frequencia_cardiaca = 80;
volatile int glicemia = 100;

// botoões
// pino do botão A
#define BTN_A_PIN 5
#define BTN_B_PIN 6

volatile bool a_state = false; // Botao A está pressionado?
volatile bool b_state = false; // Botao A está pressionado?

uint32_t last_button_press_time = 0;
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

// inicializa o led
void setup_led()
{
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);
}

// inicializa os botões
void setup_botoes()
{
    // INICIANDO BOTÄO A
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    // INICIANDO BOTÄO B
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);
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

// Sub Menu para exibir após escolher uma função
void exibe_ajuste(int valor, char *nome)
{
    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    char buffer[22] = {0};
    snprintf(buffer, sizeof(buffer), "- %d +", valor);
    ssd1306_draw_string(ssd, 10, 10, "ajuste o valor");
    ssd1306_draw_string(ssd, 40, 20, nome);
    ssd1306_draw_string(ssd, 30, 42, buffer);

    render_on_display(ssd, &frame_area);
}

// Função de ajuste de frequencia cardiaca
void ajustar_batimento()
{
    uint16_t vrx_value, vry_value;
    joystick_read_axis(&vrx_value, &vry_value);

    // Ajusta a frequência cardíaca baseado no valor do analógico
    if (vrx_value > 3000)
    {
        frequencia_cardiaca += 5;
    }
    else if (vrx_value < 1000)
    {
        frequencia_cardiaca -= 5;
    }

    // Limita a faixa da frequência cardíaca
    if (frequencia_cardiaca < 40)
        frequencia_cardiaca = 40;
    if (frequencia_cardiaca > 180)
        frequencia_cardiaca = 180;
}

// Função de ajuste de glicemia
int ajustar_glicemia()
{
    uint16_t vrx_value, vry_value;
    joystick_read_axis(&vrx_value, &vry_value);

    // Ajusta a frequência cardíaca baseado no valor do analógico
    if (vrx_value > 3000)
    {
        glicemia += 5;
    }
    else if (vrx_value < 1000)
    {
        glicemia -= 5;
    }

    // Limita a faixa da frequência cardíaca
    if (glicemia < 50)
        glicemia = 50;
    if (glicemia > 160)
        glicemia = 160;
}

void finalizar_menu()
{
    // tudo
}

void execute_selection()
{
    sleep_ms(300);
    switch (menu_index)
    {
    case 0:
        // ajustar batimento
        while (!b_state)
        {
            exibe_ajuste(frequencia_cardiaca, "BPM");
            ajustar_batimento();
            sleep_ms(300);
            if (!gpio_get(BTN_B_PIN)) // Verifica se o botão foi pressionada
            {
                b_state = true;
            }
        }
        break;
    case 1:
        // ajustar glicemia
        while (!b_state)
        {
            exibe_ajuste(glicemia, "GLC");
            ajustar_glicemia();
            sleep_ms(300);
            if (!gpio_get(BTN_B_PIN)) // Verifica se o botão foi pressionada
            {
                b_state = true;
            }
        }
        break;
    }
    b_state = false;
}

void espera_com_leitura(int timeMS)
{
    for (int i = 0; i < timeMS; i += 100)
    {
        if (!gpio_get(BTN_A_PIN)) // Verifica se o botão foi pressionado
        {
            a_state = false;
        }
        sleep_ms(100);
    }
}

// apaga todos os Led
void apagar_led(int intervalo)
{
    espera_com_leitura(intervalo);
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
    espera_com_leitura(intervalo);
}

// faz a função de monitorar (pisca o led de acordo com a frenquecia e na cor do nivel de glicose)
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

// mostra no display a frequencia cardiaca e a glicose
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

// Função para conectar ao Wi-Fi
int wifi_connect()
{

    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};

    // Inicializa o Wi-Fi
    if (cyw43_arch_init())
    {
        calculate_render_area_buffer_length(&frame_area);
        uint8_t ssd[ssd1306_buffer_length];
        memset(ssd, 0, ssd1306_buffer_length);
        ssd1306_draw_string(ssd, 5, 20, "Erro ao iniciar");
        ssd1306_draw_string(ssd, 5, 40, "   o Wi-Fi");
        render_on_display(ssd, &frame_area);
        sleep_ms(2000);
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    ssd1306_draw_string(ssd, 5, 20, "Conectando WiFi");
    ssd1306_draw_string(ssd, 5, 40, "Aguarde...");
    render_on_display(ssd, &frame_area);
    sleep_ms(2000);

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        calculate_render_area_buffer_length(&frame_area);
        uint8_t ssd[ssd1306_buffer_length];
        memset(ssd, 0, ssd1306_buffer_length);
        ssd1306_draw_string(ssd, 5, 20, "Falha na");
        ssd1306_draw_string(ssd, 5, 30, "conexão");
        render_on_display(ssd, &frame_area);
        sleep_ms(2000);
        return 1;
    }
    else
    {
        calculate_render_area_buffer_length(&frame_area);
        uint8_t ssd[ssd1306_buffer_length];
        memset(ssd, 0, ssd1306_buffer_length);
        ssd1306_draw_string(ssd, 5, 10, "Conectado");

        // Read the ip address in a human readable way
        uint8_t *ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
        ssd1306_draw_string(ssd, 5, 24, "Endereço IP ");
        char buffer[22] = {0};
        snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
        ssd1306_draw_string(ssd, 5, 32, buffer);
        render_on_display(ssd, &frame_area);
        sleep_ms(5000);
        return 0;
    }
}

int main()
{
    setup_joystick();
    init_oled();
    setup_botoes();
    setup_led();

    int conectado = 1;
    // tenta conectar ate conseguir
    if (conectado == 1)
    {
        conectado = wifi_connect();
    }
    
    display_menu();

    while (true)
    {
        // ativa o menu para fazer configuração inicial dos inputs
        while (!a_state)
        {
            navigate_menu();
            if (!gpio_get(SW)) // Verifica se o botão foi pressionada
            {
                execute_selection();
                sleep_ms(300);
                display_menu();
            }
            if (!gpio_get(BTN_A_PIN)) // para sair do menu e iniciar a rotina
            {
                a_state = true;
            }
        }

        sleep_ms(300);

        // rotina normal
        while (a_state)
        {
            // mostra no display a frequencia cardiaca e a glicose
            exibe_situcao(frequencia_cardiaca, glicemia);

            // faz a função de monitorar (pisca o led de acordo com a frenquecia e na cor do nivel de glicose)
            monitoramento(glicemia, frequencia_cardiaca);
        }
    }
}