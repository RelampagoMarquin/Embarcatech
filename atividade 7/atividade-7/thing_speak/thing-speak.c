#include "thing-speak.h"

uint8_t *ip_address;

// Variáveis globais para armazenar os dados e a resposta do ThingSpeak
static int bpm = 0;
static int glc = 0;
static char thingspeak_response[512] = "Aguardando resposta do servidor...";


bool is_wifi_connected() {
    // Verifica se o estado da interface é "UP" (ativo e conectado)
    return netif_is_link_up(&cyw43_state.netif[0]);
}
  
// Função para conectar ao Wi-Fi
int conectar_wifi(uint8_t width, uint8_t n_pages, int buffer_length)
{

    struct render_area frame_area = {0, width - 1, 0, n_pages - 1};

    // Inicializa o Wi-Fi
    if (cyw43_arch_init())
    {
        calculate_render_area_buffer_length(&frame_area);
        uint8_t ssd[buffer_length];
        memset(ssd, 0, buffer_length);
        ssd1306_draw_string(ssd, 5, 20, "Erro ao iniciar");
        ssd1306_draw_string(ssd, 5, 40, "   o Wi-Fi");
        render_on_display(ssd, &frame_area);
        sleep_ms(2000);
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[buffer_length];
    memset(ssd, 0, buffer_length);
    ssd1306_draw_string(ssd, 5, 20, "Conectando WiFi");
    ssd1306_draw_string(ssd, 5, 40, "Aguarde...");
    render_on_display(ssd, &frame_area);
    sleep_ms(2000);

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 20000))
    {
        calculate_render_area_buffer_length(&frame_area);
        uint8_t ssd[buffer_length];
        memset(ssd, 0, buffer_length);
        ssd1306_draw_string(ssd, 5, 20, "Falha ao");
        ssd1306_draw_string(ssd, 5, 30, "conectar");
        render_on_display(ssd, &frame_area);
        sleep_ms(2000);
        return 1;
    }
    else
    {
        calculate_render_area_buffer_length(&frame_area);
        uint8_t ssd[buffer_length];
        memset(ssd, 0, buffer_length);
        ssd1306_draw_string(ssd, 5, 10, "Conectado");

        // Read the ip address in a human readable way
        ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
        ssd1306_draw_string(ssd, 5, 24, "Endereço IP ");
        char buffer[22] = {0};
        snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
        ssd1306_draw_string(ssd, 5, 32, buffer);
        render_on_display(ssd, &frame_area);
        sleep_ms(5000);
        return 0;
    }
}

// Callback para processar a resposta do servidor
static err_t receive_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    // Armazena a resposta para exibição no servidor HTTP
    snprintf(thingspeak_response, sizeof(thingspeak_response), "Resposta do ThingSpeak: %.*s", p->len, (char *)p->payload);
    
    // Liberar o buffer e fechar conexão
    pbuf_free(p);
    tcp_close(tpcb);
    return ERR_OK;
}


// Função para enviar dados para o ThingSpeak e receber resposta
void send_data_to_thingspeak(int bpm, int glc) {
    char post_data[256];

    // Formato correto: "api_key=XXXX&field1=XX&field2=XX"
    snprintf(post_data, sizeof(post_data),
             "api_key=%s&field1=%d&field2=%d",
             API_KEY, bpm, glc);

    // Criar conexão TCP
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB TCP\n");
        return;
    }

    // Endereço IP do ThingSpeak (api.thingspeak.com)
    ip_addr_t ip;
    IP4_ADDR(&ip, 184, 106, 153, 149);

    // Conectar ao servidor na porta 80
    err_t err = tcp_connect(pcb, &ip, 80, NULL);
    if (err != ERR_OK) {
        printf("Erro ao conectar ao ThingSpeak\n");
        tcp_close(pcb);
        return;
    }

    // Montar a requisição HTTP
    char request[512];
    snprintf(request, sizeof(request),
             "GET /update.json HTTP/1.1\r\n"
             "Host: api.thingspeak.com\r\n"
             "Connection: close\r\n"
             "Content-Type: application/x-www-form-urlencoded\r\n"
             "Content-Length: %d\r\n\r\n"
             "%s",
             (int)strlen(post_data), post_data);

    // Enviar a requisição
    err_t send_err = tcp_write(pcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    if (send_err != ERR_OK) {
        printf("Erro ao enviar os dados para o ThingSpeak\n");
        tcp_close(pcb);
        return;
    } else{
        printf("Dados enviados para ThingSpeak\n");        
    }

    // Garantir que os dados sejam enviados
    tcp_output(pcb);

    // Registrar callback para receber resposta
    tcp_recv(pcb, receive_callback);
}