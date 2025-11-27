/**
 * @file    ws2812.h
 * @brief   WS2812 LED Matrix Driver for BitDogLab
 * @details Driver using PIO for precise timing control of WS2812B LEDs
 * 
 * @project BitDogLab_HTTPDd_workspace
 * @url     https://github.com/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace
 * 
 * @author  Carlos Delfino
 * @email   consultoria@carlosdelfino.eti.br
 * @website https://carlosdelfino.eti.br
 * @github  https://github.com/CarlosDelfino
 * 
 * @license CC BY 4.0 - https://creativecommons.org/licenses/by/4.0/
 */

#ifndef WS2812_H
#define WS2812_H

#include "pico/stdlib.h"
#include "hardware/pio.h"

#ifdef __cplusplus
extern "C" {
#endif

// WS2812 Configuration
#define WS2812_DEFAULT_PIN      7
#define WS2812_DEFAULT_NUM_LEDS 25
#define WS2812_FREQ             800000  // 800kHz

/**
 * Initialize WS2812 driver using PIO
 * 
 * @param pin GPIO pin connected to WS2812 data line
 * @param num_leds Number of LEDs in the strip/matrix
 * @return true if initialization successful
 */
bool ws2812_init(uint pin, uint num_leds);

/**
 * Send color data to WS2812 LEDs
 * Colors are in RGB format (0xRRGGBB), will be converted to GRB internally
 * 
 * @param colors Array of RGB colors (0xRRGGBB format)
 * @param num_leds Number of LEDs to update
 */
void ws2812_send(const uint32_t *colors, uint num_leds);

/**
 * Set all LEDs to a single color
 * 
 * @param color RGB color (0xRRGGBB format)
 * @param num_leds Number of LEDs to set
 */
void ws2812_fill(uint32_t color, uint num_leds);

/**
 * Turn off all LEDs
 * 
 * @param num_leds Number of LEDs to clear
 */
void ws2812_clear(uint num_leds);

/**
 * Convert XY coordinates to LED index for 5x5 serpentine matrix
 * 
 * @param x Column (0-4)
 * @param y Row (0-4)
 * @return LED index (0-24)
 */
static inline uint8_t ws2812_xy_to_index(uint8_t x, uint8_t y) {
    if (y % 2 == 0) {
        // Even row: left to right
        return y * 5 + x;
    } else {
        // Odd row: right to left
        return y * 5 + (4 - x);
    }
}

/**
 * Convert LED index to XY coordinates
 * 
 * @param index LED index (0-24)
 * @param x Pointer to store column (0-4)
 * @param y Pointer to store row (0-4)
 */
static inline void ws2812_index_to_xy(uint8_t index, uint8_t *x, uint8_t *y) {
    *y = index / 5;
    if (*y % 2 == 0) {
        *x = index % 5;
    } else {
        *x = 4 - (index % 5);
    }
}

/**
 * Convert RGB to GRB format (WS2812 native format)
 * 
 * @param rgb RGB color (0xRRGGBB)
 * @return GRB color for WS2812
 */
static inline uint32_t ws2812_rgb_to_grb(uint32_t rgb) {
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
}

#ifdef __cplusplus
}
#endif

#endif // WS2812_H
