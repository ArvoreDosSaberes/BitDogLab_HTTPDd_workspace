/**
 * @file    ws2812.c
 * @brief   WS2812 LED Matrix Driver for BitDogLab
 * @details Uses PIO for precise timing control of WS2812B LEDs
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

#include "ws2812.h"
#include "ws2812.pio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// PIO instance and state machine
static PIO ws2812_pio = pio0;
static uint ws2812_sm = 0;
static uint ws2812_offset = 0;
static bool ws2812_initialized = false;

bool ws2812_init(uint pin, uint num_leds) {
    if (ws2812_initialized) {
        return true;  // Already initialized
    }
    
    // Try to claim a state machine
    int sm = pio_claim_unused_sm(ws2812_pio, false);
    if (sm < 0) {
        // Try pio1 if pio0 is full
        ws2812_pio = pio1;
        sm = pio_claim_unused_sm(ws2812_pio, false);
        if (sm < 0) {
            return false;
        }
    }
    ws2812_sm = (uint)sm;
    
    // Add program to PIO
    if (!pio_can_add_program(ws2812_pio, &ws2812_program)) {
        pio_sm_unclaim(ws2812_pio, ws2812_sm);
        return false;
    }
    ws2812_offset = pio_add_program(ws2812_pio, &ws2812_program);
    
    // Initialize the program
    ws2812_program_init(ws2812_pio, ws2812_sm, ws2812_offset, pin, WS2812_FREQ, false);
    
    ws2812_initialized = true;
    
    // Clear all LEDs on init
    ws2812_clear(num_leds);
    
    return true;
}

static inline void ws2812_put_pixel(uint32_t grb) {
    pio_sm_put_blocking(ws2812_pio, ws2812_sm, grb << 8u);
}

void ws2812_send(const uint32_t *colors, uint num_leds) {
    if (!ws2812_initialized) {
        return;
    }
    
    for (uint i = 0; i < num_leds; i++) {
        // Convert RGB to GRB format
        uint32_t grb = ws2812_rgb_to_grb(colors[i]);
        ws2812_put_pixel(grb);
    }
    
    // Wait for reset time (>50us)
    //sleep_us(60);
}

void ws2812_fill(uint32_t color, uint num_leds) {
    if (!ws2812_initialized) {
        return;
    }
    
    uint32_t grb = ws2812_rgb_to_grb(color);
    
    for (uint i = 0; i < num_leds; i++) {
        ws2812_put_pixel(grb);
    }
    
    sleep_us(60);
}

void ws2812_clear(uint num_leds) {
    ws2812_fill(0x000000, num_leds);
}
