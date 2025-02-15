#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>

#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "lwip/tcp.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define LED_PIN 12
#define BUZZER_PIN 21


#define BUZZER_FREQUENCY 1000
#define DISPLAY_PIN 22
#define SSID "YOUR_SSID"
#define PASSWORD "YOUR_PASSWORD"
#define HTTP_RESPONSE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" \
                      "<!DOCTYPE html><html><body>" \
                      "<h1>Pagina para emitir um alerta</h1>" \
                      "<p><a href=\"/ligaralerta\">emitir um alerta </a></p>" \
                      "</body></html>\r\n"
int activeAlert = 0; // flag para ativar o alerta
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

const uint frequency[] = {1000,1000,1000,1000,1000,1000,
    1000,1000,1000,1000,1000,1000,
    1000,1000,1000,1000,1000,1000,
    1000,1000,1000,1000,1000,1000,

};


void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f); // Ajusta divisor de clock
    pwm_init(slice_num, &config, true);
}



void play_alert(uint pin, uint frequency) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint32_t clock_freq = clock_get_hz(clk_sys);
    uint32_t top = clock_freq / frequency - 1;

    pwm_set_wrap(slice_num, top);
    pwm_set_gpio_level(pin,  (top * 3) / 4); // 50% de duty cycle

    sleep_ms(500);

    pwm_set_gpio_level(pin, 0); // Desliga o som após a duração
    sleep_ms(50); // Pausa entre notas
   
 
}

// Função de callback para processar requisições HTTP
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        // Cliente fechou a conexão
        tcp_close(tpcb);
        return ERR_OK;
    }

    // Processa a requisição HTTP
    char *request = (char *)p->payload;

    if (strstr(request, "GET /ligaralerta")) {
        activeAlert=1;
          // Liga o LED
        printf("flask mudou para 1");
    } 


    // Envia a resposta HTTP
    tcp_write(tpcb, HTTP_RESPONSE, strlen(HTTP_RESPONSE), TCP_WRITE_FLAG_COPY);

    // Libera o buffer recebido
    pbuf_free(p);

    return ERR_OK;
}

// Callback de conexão: associa o http_callback à conexão
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);  // Associa o callback HTTP
    return ERR_OK;
}

// Função de setup do servidor TCP
static void start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        return;
    }

    // Liga o servidor na porta 80
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }

    pcb = tcp_listen(pcb);  // Coloca o PCB em modo de escuta
    tcp_accept(pcb, connection_callback);  // Associa o callback de conexão

    printf("Servidor HTTP rodando na porta 80...\n");
}

int main() {
    stdio_init_all();
    sleep_ms(10000);

    set_sys_clock_khz(125000, true); // Ajusta o clock do sistema para 125 MHz

    printf("Iniciando servidor HTTP\n");

    // Inicializa o Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar o Wi-Fi\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(SSID, PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha ao conectar ao Wi-Fi\n");
        return 1;
    }else {
        printf("Connected.\n");
        // Read the ip address in a human readable way
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        printf("Endereço IP %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }

    printf("Wi-Fi conectado!\n");
    printf("Para ligar ou desligar o LED acesse o Endereço IP seguido de /led/on ou /led/off\n");

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    start_http_server();

    printf("configurando o display");
    sleep_ms(10000);

    i2c_init(i2c1, ssd1306_i2c_clock * 1000); // Inicializa o I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();

    sleep_ms(1000);
    pwm_init_buzzer(BUZZER_PIN); // Inicializa o buzzer

    sleep_ms(1000);

    while (true) {
        cyw43_arch_poll();  // Necessário para manter o Wi-Fi ativo
        sleep_ms(100);
        
    if(activeAlert==1){
        struct render_area frame_area = {
            start_column : 0,
            end_column : ssd1306_width - 1,
            start_page : 0,
            end_page : ssd1306_n_pages - 1
        };
  
        uint8_t ssd[ssd1306_buffer_length];
        memset(ssd, 0, ssd1306_buffer_length);

        char *text = "Sinal de alerta!";
        ssd1306_draw_string(ssd, 5, 10, text);

      calculate_render_area_buffer_length(&frame_area);
      render_on_display(ssd, &frame_area); // Renderiza a tela

        for(int i=0;i< sizeof(frequency) / sizeof(frequency[0]);i++){
            gpio_put(LED_PIN, 1);
            play_alert(BUZZER_PIN,frequency[i]);
            sleep_ms(1000);
            gpio_put(LED_PIN, 0);
            sleep_ms(1000);
        }
      

    }

    else if(activeAlert==0){
        sleep_ms(1000);
        struct render_area frame_area = {
            start_column : 0,
            end_column : ssd1306_width - 1,
            start_page : 0,
            end_page : ssd1306_n_pages - 1
        };
  
        uint8_t ssd[ssd1306_buffer_length];
        memset(ssd, 0, ssd1306_buffer_length);

        char *text = "Sinal estavel";
        ssd1306_draw_string(ssd, 5, 10, text);

      calculate_render_area_buffer_length(&frame_area);
      render_on_display(ssd, &frame_area);
    }
}
    cyw43_arch_deinit(); 


    

    return 0;
}