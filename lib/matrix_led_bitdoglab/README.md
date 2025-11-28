# Matriz de LEDs WS2818B

Matriz de LEDs endereçáveis RGB do tipo WS2818B (NeoPixel), permitindo controle individual de cada LED.

| Função     | GPIO | Tipo        | Observação                    |
|------------|------|-------------|-------------------------------|
| Data Line  | 7    | Output      | Protocolo WS2818B (800kHz)    |

**Especificações:**
- **Tipo:** WS2818B (LEDs RGB endereçáveis)
- **Protocolo:** Dados seriais em cadeia (daisy-chain)
- **Tensão:** 5V (com tolerância para 3.3V de dados)
- **Biblioteca recomendada:** PIO-based WS2812 driver

```c
// Exemplo de definição
#define WS2818B_PIN 7
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

