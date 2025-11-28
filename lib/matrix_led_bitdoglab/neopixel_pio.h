#ifndef NEOPIXEL_PIO_H
#define NEOPIXEL_PIO_H

#include "pico/stdlib.h"

// Definição do número de LEDs e pino.
#define LED_COUNT 25
#define LED_PIN 7



// Funções públicas
void npInit(uint pin);
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);
void npClear(void);
void npWrite(void);

#endif // NEOPIXEL_PIO_H