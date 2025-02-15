# Projeto: Servidor HTTP com Alerta Sonoro e Display OLED na Raspberry Pi Pico W

## Descrição
Este projeto implementa um **servidor HTTP** em uma **Raspberry Pi Pico W** que permite ativar um **alerta sonoro** e exibir mensagens em um **display OLED SSD1306** via Wi-Fi. O alerta é composto por um **LED piscante e um buzzer** tocando uma sequência de frequências predefinidas.

## Funcionalidades
- Servidor HTTP rodando na **porta 80**.
- Conexão com uma rede Wi-Fi usando credenciais definidas no código.
- Endpoint HTTP para ativar o alerta: **`/ligaralerta`**.
- Controle de LED (piscando durante o alerta).
- Emissão de som usando **PWM** no buzzer.
- Exibição de mensagens no **display OLED SSD1306 via I2C**.

---

## Hardware Necessário
- **Raspberry Pi Pico W**
- **Módulo OLED SSD1306 (I2C)**
- **LED** (conectado ao pino 12)
- **Buzzer** (conectado ao pino 21)
- **Resistores e jumpers para conexões**

---

## Pinos Utilizados
| Componente | Pino na Pico W |
|------------|---------------|
| LED        | GPIO 12       |
| Buzzer     | GPIO 21       |
| Display OLED SDA | GPIO 14 |
| Display OLED SCL | GPIO 15 |

---

## Bibliotecas Utilizadas
- `pico/stdlib.h` - Funções padrão para a Raspberry Pi Pico.
- `hardware/gpio.h` - Controle de GPIOs.
- `pico/cyw43_arch.h` - Biblioteca para conexão Wi-Fi.
- `hardware/pwm.h` - Controle de PWM para o buzzer.
- `hardware/clocks.h` - Configuração de clocks da Pico W.
- `lwip/tcp.h` - Implementação de servidor HTTP via TCP.
- `ssd1306.h` - Controle do display OLED via I2C.

---

## Configuração do Wi-Fi
No código, substitua `YOUR_SSID` e `YOUR_PASSWORD` pelas credenciais da sua rede:
```c
#define SSID "YOUR_SSID"
#define PASSWORD "YOUR_PASSWORD"
```

---

## Funcionamento do Código

### Inicialização
1. Configura **GPIOs**, **I2C** e **Wi-Fi**.
2. Conecta-se à rede Wi-Fi.
3. Exibe o **endereço IP** obtido via DHCP.
4. Inicializa o **servidor HTTP** na porta 80.

### Servidor HTTP
- Quando um cliente acessa a URL **`/ligaralerta`**, o sistema:
  1. Atualiza a flag `activeAlert=1`.
  2. Exibe "Sinal de alerta!" no **display OLED**.
  3. Ativa o **LED piscante**.
  4. Toca um som de alerta no **buzzer**.
  5. Aguarda e reinicia o ciclo até o estado ser alterado.

### Controle do Buzzer
- Usa **PWM** para gerar um sinal sonoro.
- Frequência definida em um array:
  ```c
  const uint frequency[] = {1000, 1000, 1000, ...};
  ```
- Alterna entre `ON` e `OFF` para criar o alerta sonoro.

### Exibição no Display OLED
- Utiliza a biblioteca `ssd1306.h` para exibir mensagens.
- Mostra "Sinal estável" quando `activeAlert=0`.
- Mostra "Sinal de alerta!" quando `activeAlert=1`.

---

## Como Usar
1. **Compile e carregue o código** na Raspberry Pi Pico W.
2. **Acesse o endereço IP** exibido no terminal.
3. **Clique em "Emitir um alerta"** para ativar o alerta.
4. O **LED piscará**, o **buzzer tocará** e o **display exibirá "Sinal de alerta!"**.

---

## Possíveis Melhorias
- Implementar um endpoint para **desativar o alerta remotamente**.
- Criar uma **página HTML interativa** para controle.
- Melhorar o design do **sinal sonoro**.
- Adicionar suporte a **sensores externos** para disparo automático.

---

## Conclusão
Este projeto demonstra o uso da Raspberry Pi Pico W como um **servidor IoT**, controlando componentes via Wi-Fi. Ele pode ser adaptado para diversas aplicações, como **sistemas de alerta, monitoramento e automação residencial**.


