# BitDogLab - Mapeamento de Pinos da RP2040

A **BitDogLab** é uma placa educacional baseada no microcontrolador **RP2040** (Raspberry Pi Pico W), projetada para facilitar o aprendizado de sistemas embarcados e IoT. Este documento descreve a pinagem dos periféricos disponíveis na placa.

---

## Resumo de Pinos Utilizados

| Pino GPIO | Função                  | Direção        |
|-----------|-------------------------|----------------|
| 5         | Botão A                 | Input Pull-up  |
| 6         | Botão B                 | Input Pull-up  |
| 7         | Matriz LED WS2812B      | Output (Data)  |
| 10        | Buzzer Direito          | Output (PWM)   |
| 11        | LED RGB - Verde         | Output (PWM)   |
| 12        | LED RGB - Azul          | Output (PWM)   |
| 13        | LED RGB - Vermelho      | Output (PWM)   |
| 14        | I2C1 SDA (OLED)         | I2C            |
| 15        | I2C1 SCL (OLED)         | I2C            |
| 21        | Buzzer Esquerdo         | Output (PWM)   |
| 22        | Joystick - Botão        | Input Pull-up  |
| 26        | Joystick - Eixo X       | Input ADC0     |
| 27        | Joystick - Eixo Y       | Input ADC1     |
| 28        | ADC2 (Expansão)         | Input ADC2     |

---

## LED RGB

LED RGB de ânodo comum localizado na placa, controlável via PWM para mistura de cores.

| Cor       | GPIO | Tipo           | Observação                     |
|-----------|------|----------------|--------------------------------|
| Vermelho  | 13   | Output (PWM)   | Ativo em nível baixo (LOW)     |
| Verde     | 11   | Output (PWM)   | Ativo em nível baixo (LOW)     |
| Azul      | 12   | Output (PWM)   | Ativo em nível baixo (LOW)     |

**Nota:** Por ser ânodo comum, o LED acende quando o pino está em nível LOW. Para controle de intensidade via PWM, valores menores = maior brilho.

```c
// Exemplo de definição
#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12
```

---

## Botões

Dois botões tácteis com resistores de pull-up internos já habilitados na placa.

| Botão   | GPIO | Tipo           | Estado Pressionado |
|---------|------|----------------|-------------------|
| Botão A | 5    | Input Pull-up  | LOW (0)           |
| Botão B | 6    | Input Pull-up  | LOW (0)           |

**Nota:** Os botões retornam `0` quando pressionados e `1` quando soltos devido ao pull-up.

```c
// Exemplo de definição
#define BTN_A_PIN 5
#define BTN_B_PIN 6
```

---

## Buzzers

Dois buzzers piezoelétricos para geração de tons e efeitos sonoros. Podem ser controlados via PWM para diferentes frequências.

| Buzzer    | GPIO | Tipo         | Observação              |
|-----------|------|--------------|-------------------------|
| Esquerdo  | 21   | Output (PWM) | Buzzer passivo          |
| Direito   | 10   | Output (PWM) | Buzzer passivo          |

**Nota:** Por serem buzzers passivos, é necessário gerar um sinal PWM na frequência desejada para produzir som.

```c
// Exemplo de definição
#define BUZZER_LEFT_PIN  21
#define BUZZER_RIGHT_PIN 10
```

---

## Matriz de LEDs WS2812B

Matriz de LEDs endereçáveis RGB do tipo WS2812B (NeoPixel), permitindo controle individual de cada LED.

| Função     | GPIO | Tipo        | Observação                    |
|------------|------|-------------|-------------------------------|
| Data Line  | 7    | Output      | Protocolo WS2812B (800kHz)    |

**Especificações:**
- **Tipo:** WS2812B (LEDs RGB endereçáveis)
- **Protocolo:** Dados seriais em cadeia (daisy-chain)
- **Tensão:** 5V (com tolerância para 3.3V de dados)
- **Biblioteca recomendada:** PIO-based WS2812 driver

```c
// Exemplo de definição
#define WS2812_PIN 7
#define NUM_LEDS   25  // Matriz 5x5
```

### Endereçamento da Matriz 5x5

A matriz possui **25 LEDs** organizados em **5 linhas × 5 colunas**. Os LEDs são conectados em série (daisy-chain) e endereçados de **0 a 24** em um padrão **serpentina (zigzag)**.

#### Mapa de Endereços (Vista Frontal)

```
    Coluna:   0     1     2     3     4
           ┌─────┬─────┬─────┬─────┬─────┐
  Linha 0  │  0  │  1  │  2  │  3  │  4  │  → (esquerda para direita)
           ├─────┼─────┼─────┼─────┼─────┤
  Linha 1  │  9  │  8  │  7  │  6  │  5  │  ← (direita para esquerda)
           ├─────┼─────┼─────┼─────┼─────┤
  Linha 2  │ 10  │ 11  │ 12  │ 13  │ 14  │  → (esquerda para direita)
           ├─────┼─────┼─────┼─────┼─────┤
  Linha 3  │ 19  │ 18  │ 17  │ 16  │ 15  │  ← (direita para esquerda)
           ├─────┼─────┼─────┼─────┼─────┤
  Linha 4  │ 20  │ 21  │ 22  │ 23  │ 24  │  → (esquerda para direita)
           └─────┴─────┴─────┴─────┴─────┘
                      ↑
               Pino de Dados (GPIO 7)
```

#### Padrão Serpentina Explicado

- **Linhas pares (0, 2, 4):** Endereços crescem da esquerda → direita
- **Linhas ímpares (1, 3):** Endereços crescem da direita → esquerda

Este padrão é comum em matrizes de LEDs e otimiza o layout da PCB.

### Conversão Coordenadas (X,Y) → Índice

```c
// Converte coordenadas (x, y) para índice do LED
// x = coluna (0-4), y = linha (0-4)
uint8_t xy_to_index(uint8_t x, uint8_t y) {
    if (y % 2 == 0) {
        // Linha par: esquerda para direita
        return y * 5 + x;
    } else {
        // Linha ímpar: direita para esquerda
        return y * 5 + (4 - x);
    }
}

// Converte índice para coordenadas (x, y)
void index_to_xy(uint8_t index, uint8_t *x, uint8_t *y) {
    *y = index / 5;
    if (*y % 2 == 0) {
        *x = index % 5;
    } else {
        *x = 4 - (index % 5);
    }
}
```

### Tabela de Referência Rápida

| Coord (X,Y) | Índice | Coord (X,Y) | Índice | Coord (X,Y) | Índice |
|-------------|--------|-------------|--------|-------------|--------|
| (0,0)       | 0      | (0,1)       | 9      | (0,2)       | 10     |
| (1,0)       | 1      | (1,1)       | 8      | (1,2)       | 11     |
| (2,0)       | 2      | (2,1)       | 7      | (2,2)       | 12     |
| (3,0)       | 3      | (3,1)       | 6      | (3,2)       | 13     |
| (4,0)       | 4      | (4,1)       | 5      | (4,2)       | 14     |
| (0,3)       | 19     | (0,4)       | 20     |             |        |
| (1,3)       | 18     | (1,4)       | 21     |             |        |
| (2,3)       | 17     | (2,4)       | 22     |             |        |
| (3,3)       | 16     | (3,4)       | 23     |             |        |
| (4,3)       | 15     | (4,4)       | 24     |             |        |

### Exemplos de Símbolos e Imagens

#### Definindo Cores

```c
// Cores básicas (formato GRB - ordem do WS2812B)
#define COLOR_OFF     0x000000
#define COLOR_RED     0x00FF00  // G=0, R=FF, B=0
#define COLOR_GREEN   0xFF0000  // G=FF, R=0, B=0
#define COLOR_BLUE    0x0000FF  // G=0, R=0, B=FF
#define COLOR_WHITE   0xFFFFFF
#define COLOR_YELLOW  0xFFFF00  // G=FF, R=FF, B=0
#define COLOR_CYAN    0xFF00FF  // G=FF, R=0, B=FF
#define COLOR_MAGENTA 0x00FFFF  // G=0, R=FF, B=FF
```

#### Símbolo: Coração ❤️

```
    ·  ■  ·  ■  ·       Índices acesos:
    ■  ■  ■  ■  ■       1, 3, 5, 6, 7, 8, 9,
    ■  ■  ■  ■  ■       10, 11, 12, 13, 14,
    ·  ■  ■  ■  ·       17, 18, 19, 22
    ·  ·  ■  ·  ·
```

```c
const uint8_t heart[] = {1, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17, 18, 19, 22};
const uint8_t heart_size = sizeof(heart);

void draw_heart(uint32_t *leds, uint32_t color) {
    // Limpa todos os LEDs
    for (int i = 0; i < 25; i++) leds[i] = COLOR_OFF;
    // Acende os LEDs do coração
    for (int i = 0; i < heart_size; i++) {
        leds[heart[i]] = color;
    }
}
```

#### Símbolo: Smile 😊

```
    ·  ■  ·  ■  ·       Índices acesos:
    ·  ·  ·  ·  ·       1, 3 (olhos)
    ■  ·  ·  ·  ■       10, 14 (cantos da boca)
    ·  ■  ■  ■  ·       17, 18, 19 (boca)
    ·  ·  ·  ·  ·
```

```c
const uint8_t smile[] = {1, 3, 10, 14, 17, 18, 19};
const uint8_t smile_size = sizeof(smile);
```

#### Símbolo: X (Erro) ✗

```
    ■  ·  ·  ·  ■       Índices acesos:
    ·  ■  ·  ■  ·       0, 4, 6, 8, 12,
    ·  ·  ■  ·  ·       16, 18, 20, 24
    ·  ■  ·  ■  ·
    ■  ·  ·  ·  ■
```

```c
const uint8_t x_symbol[] = {0, 4, 6, 8, 12, 16, 18, 20, 24};
const uint8_t x_symbol_size = sizeof(x_symbol);
```

#### Símbolo: Check (OK) ✓

```
    ·  ·  ·  ·  ■       Índices acesos:
    ·  ·  ·  ■  ·       4, 6, 12, 18, 20
    ·  ·  ■  ·  ·
    ■  ■  ·  ·  ·
    ·  ·  ·  ·  ·
```

```c
const uint8_t check[] = {4, 6, 12, 18, 20};
const uint8_t check_size = sizeof(check);
```

#### Símbolo: Seta para Cima ↑

```
    ·  ·  ■  ·  ·       Índices acesos:
    ·  ■  ■  ■  ·       2, 7, 8, 9, 12, 17, 22
    ·  ·  ■  ·  ·
    ·  ·  ■  ·  ·
    ·  ·  ■  ·  ·
```

```c
const uint8_t arrow_up[] = {2, 7, 8, 9, 12, 17, 22};
```

### Usando Bitmap 5x5 para Desenhar

Forma mais intuitiva de criar imagens usando uma matriz visual:

```c
// Cada bit representa um LED: 1 = aceso, 0 = apagado
// Leitura: linha por linha, da esquerda para direita
const uint8_t bitmap_heart[5] = {
    0b01010,  // Linha 0: ·■·■·
    0b11111,  // Linha 1: ■■■■■
    0b11111,  // Linha 2: ■■■■■
    0b01110,  // Linha 3: ·■■■·
    0b00100   // Linha 4: ··■··
};

// Função para renderizar bitmap na matriz
void render_bitmap(uint32_t *leds, const uint8_t *bitmap, uint32_t color) {
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            uint8_t bit = (bitmap[y] >> (4 - x)) & 0x01;
            uint8_t idx = xy_to_index(x, y);
            leds[idx] = bit ? color : COLOR_OFF;
        }
    }
}

// Uso:
// uint32_t leds[25];
// render_bitmap(leds, bitmap_heart, COLOR_RED);
```

### Animação Simples

```c
// Array de frames para animação de piscar
const uint8_t* frames[] = {bitmap_heart, bitmap_smile};
const int num_frames = 2;

void animate(uint32_t *leds, int delay_ms) {
    for (int f = 0; f < num_frames; f++) {
        render_bitmap(leds, frames[f], COLOR_RED);
        ws2812_send(leds, 25);  // Enviar para LEDs
        sleep_ms(delay_ms);
    }
}
```

### Números 0-9 (Bitmaps)

```c
const uint8_t digits[10][5] = {
    {0b01110, 0b10001, 0b10001, 0b10001, 0b01110},  // 0
    {0b00100, 0b01100, 0b00100, 0b00100, 0b01110},  // 1
    {0b01110, 0b10001, 0b00110, 0b01000, 0b11111},  // 2
    {0b11110, 0b00001, 0b01110, 0b00001, 0b11110},  // 3
    {0b10010, 0b10010, 0b11111, 0b00010, 0b00010},  // 4
    {0b11111, 0b10000, 0b11110, 0b00001, 0b11110},  // 5
    {0b01110, 0b10000, 0b11110, 0b10001, 0b01110},  // 6
    {0b11111, 0b00010, 0b00100, 0b01000, 0b01000},  // 7
    {0b01110, 0b10001, 0b01110, 0b10001, 0b01110},  // 8
    {0b01110, 0b10001, 0b01111, 0b00001, 0b01110}   // 9
};

// Exibir dígito
void show_digit(uint32_t *leds, uint8_t digit, uint32_t color) {
    if (digit > 9) return;
    render_bitmap(leds, digits[digit], color);
}
```

---

## Joystick Analógico

Joystick analógico de dois eixos com botão central integrado.

| Função   | GPIO | Tipo           | Canal ADC | Faixa de Valores |
|----------|------|----------------|-----------|------------------|
| Eixo X   | 26   | Input ADC      | ADC0      | 0 - 4095 (12-bit)|
| Eixo Y   | 27   | Input ADC      | ADC1      | 0 - 4095 (12-bit)|
| Botão    | 22   | Input Pull-up  | -         | LOW quando pressionado |

**Nota:** 
- Posição central ≈ 2048 (metade da faixa ADC)
- O botão é ativado pressionando o joystick para baixo

```c
// Exemplo de definição
#define JOYSTICK_X_PIN   26
#define JOYSTICK_Y_PIN   27
#define JOYSTICK_BTN_PIN 22

#define JOYSTICK_X_ADC   0
#define JOYSTICK_Y_ADC   1
```

---

## Display OLED

Display OLED monocromático conectado via barramento I2C.

| Função | GPIO | Tipo      | Barramento |
|--------|------|-----------|------------|
| SDA    | 14   | I2C Data  | I2C1       |
| SCL    | 15   | I2C Clock | I2C1       |

**Especificações:**
- **Interface:** I2C (I2C1)
- **Endereço I2C:** 0x3C (padrão para SSD1306)
- **Resolução:** 128x64 pixels (típico)
- **Controlador:** SSD1306
- **Frequência I2C:** Até 400kHz (Fast Mode)

```c
// Exemplo de definição
#define OLED_SDA_PIN 14
#define OLED_SCL_PIN 15
#define OLED_I2C     i2c1
#define OLED_ADDR    0x3C
```

---

## Microfone (se disponível)

Algumas versões da BitDogLab incluem um microfone MEMS para captura de áudio.

| Função | GPIO | Tipo      | Canal ADC |
|--------|------|-----------|-----------|
| Audio  | 28   | Input ADC | ADC2      |

**Nota:** Verificar disponibilidade conforme versão da placa.

---

## Conector de Expansão (14 Pinos)

Conector superior de 14 pinos para expansão e conexão de módulos externos.

### Layout do Conector (Vista Superior)

| Pino | 1   | 2   | 3   | 4   | 5   | 6   | 7   |
|------|-----|-----|-----|-----|-----|-----|-----|
| Row 1| GND | 16  | 17  | GND | 28  | 3V3 | GND |
| Row 2| 18  | 19  | 20  | 4   | 9   | 9   | 5V  |

### Detalhamento dos Pinos de Expansão

| GPIO | Funções Alternativas           | Observação           |
|------|--------------------------------|----------------------|
| 4    | SPI0 RX, I2C0 SDA, UART1 TX    | GPIO de uso geral    |
| 9    | SPI1 CSn, I2C0 SCL, UART1 RX   | Aparece 2x no conector |
| 16   | SPI0 RX, I2C0 SDA, UART0 TX    | GPIO de uso geral    |
| 17   | SPI0 CSn, I2C0 SCL, UART0 RX   | GPIO de uso geral    |
| 18   | SPI0 SCK, I2C1 SDA             | GPIO de uso geral    |
| 19   | SPI0 TX, I2C1 SCL              | GPIO de uso geral    |
| 20   | SPI0 RX, I2C0 SDA, UART1 TX    | GPIO de uso geral    |
| 28   | ADC2                           | Entrada analógica    |

### Alimentação Disponível

| Pino | Tensão | Corrente Máxima     |
|------|--------|---------------------|
| 3V3  | 3.3V   | Limitada pelo regulador |
| 5V   | 5V     | Via USB ou fonte externa |
| GND  | 0V     | Referência comum    |

---

## Diagrama de Blocos

```
                    ┌─────────────────────────────────────┐
                    │           BitDogLab                 │
                    │         (RP2040 / Pico W)           │
                    ├─────────────────────────────────────┤
                    │                                     │
    ┌───────────┐   │  ┌─────────┐    ┌──────────────┐   │
    │ Botão A   │───┼──│ GPIO 5  │    │  LED RGB     │   │
    │ Botão B   │───┼──│ GPIO 6  │    │  R:13 G:11   │   │
    └───────────┘   │  └─────────┘    │  B:12        │   │
                    │                  └──────────────┘   │
    ┌───────────┐   │  ┌─────────┐    ┌──────────────┐   │
    │ Joystick  │───┼──│ GPIO 26 │    │  Buzzers     │   │
    │ X/Y/Btn   │───┼──│ GPIO 27 │    │  L:21 R:10   │   │
    └───────────┘───┼──│ GPIO 22 │    └──────────────┘   │
                    │  └─────────┘                        │
    ┌───────────┐   │  ┌─────────┐    ┌──────────────┐   │
    │ OLED      │───┼──│ GPIO 14 │────│  I2C1        │   │
    │ SSD1306   │───┼──│ GPIO 15 │────│  SDA/SCL     │   │
    └───────────┘   │  └─────────┘    └──────────────┘   │
                    │                                     │
    ┌───────────┐   │  ┌─────────┐    ┌──────────────┐   │
    │ WS2812B   │───┼──│ GPIO 7  │    │  Matriz LED  │   │
    │ Matrix    │   │  └─────────┘    │  5x5 RGB     │   │
    └───────────┘   │                  └──────────────┘   │
                    │                                     │
                    │  ┌─────────────────────────────┐   │
                    │  │   Conector de Expansão      │   │
                    │  │   GPIO: 4,9,16,17,18,19,20  │   │
                    │  │   ADC: 28                   │   │
                    │  │   Power: 3V3, 5V, GND       │   │
                    │  └─────────────────────────────┘   │
                    └─────────────────────────────────────┘
```

---

## Definições Completas para C/C++

```c
#ifndef BITDOGLAB_PINS_H
#define BITDOGLAB_PINS_H

// ===== LED RGB (Ânodo Comum - Ativo LOW) =====
#define LED_R_PIN           13
#define LED_G_PIN           11
#define LED_B_PIN           12

// ===== Botões (Pull-up - Ativo LOW) =====
#define BTN_A_PIN           5
#define BTN_B_PIN           6

// ===== Buzzers (PWM) =====
#define BUZZER_LEFT_PIN     21
#define BUZZER_RIGHT_PIN    10

// ===== Matriz LED WS2812B =====
#define WS2812_PIN          7
#define WS2812_NUM_LEDS     25

// ===== Joystick =====
#define JOYSTICK_X_PIN      26
#define JOYSTICK_Y_PIN      27
#define JOYSTICK_BTN_PIN    22
#define JOYSTICK_X_ADC      0
#define JOYSTICK_Y_ADC      1

// ===== Display OLED (I2C1) =====
#define OLED_SDA_PIN        14
#define OLED_SCL_PIN        15
#define OLED_I2C_INST       i2c1
#define OLED_I2C_ADDR       0x3C

// ===== Expansão =====
#define EXP_GPIO_4          4
#define EXP_GPIO_9          9
#define EXP_GPIO_16         16
#define EXP_GPIO_17         17
#define EXP_GPIO_18         18
#define EXP_GPIO_19         19
#define EXP_GPIO_20         20
#define EXP_ADC_PIN         28
#define EXP_ADC_CHANNEL     2

#endif // BITDOGLAB_PINS_H
```

---

## Referências

- [Datasheet RP2040](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [Documentação BitDogLab](https://github.com/BitDogLab) *(verificar disponibilidade)*
