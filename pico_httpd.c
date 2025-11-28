/**
 * @file    pico_httpd.c
 * @brief   BitDogLab HTTP Server - Servidor HTTP para testes da placa BitDogLab
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
 * 
 * Based on Raspberry Pi Pico SDK examples
 * Original Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "RP2040_HTTPd_painel_BitDogLab/lib/matrix_led_bitdoglab/neopixel_pio.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

// Define o nível de log em tempo de compilação (3 = todos os níveis ativos)
#define LOG_LEVEL 3
#include "log_vt100.h"

#include "lwip/ip4_addr.h"
#include "lwip/apps/mdns.h"
#include "lwip/init.h"
#include "lwip/apps/httpd.h"

// OLED Display
#include "oled.h"

// WS2812 LED Matrix
#include "neopixel_pio.h"

void httpd_init(void);

// ===== BitDogLab Pin Definitions =====
#define LED_R_PIN           13
#define LED_G_PIN           11
#define LED_B_PIN           12

#define BTN_A_PIN           5
#define BTN_B_PIN           6

#define NEOPIXEL_PIN        7
#define NEOPIXEL_NUM_LEDS   25

#define JOYSTICK_X_PIN      26
#define JOYSTICK_Y_PIN      27
#define JOYSTICK_BTN_PIN    22
#define JOYSTICK_X_ADC      0
#define JOYSTICK_Y_ADC      1

#define BUZZER_LEFT_PIN     21
#define BUZZER_RIGHT_PIN    10

// ===== Global State Variables =====
static absolute_time_t wifi_connected_time;
static bool led_on = false;

// Button states (true = pressed, active LOW)
static volatile bool btn_a_pressed = false;
static volatile bool btn_b_pressed = false;
static volatile bool joy_btn_pressed = false;

// Joystick ADC values
static volatile uint16_t joystick_x = 2048;
static volatile uint16_t joystick_y = 2048;

// RGB LED values (0-255)
static uint8_t rgb_r = 0;
static uint8_t rgb_g = 0;
static uint8_t rgb_b = 0;

// OLED text lines buffer
#define OLED_MAX_LINES 8
#define OLED_MAX_CHARS 17
static char oled_lines[OLED_MAX_LINES][OLED_MAX_CHARS];
static int oled_current_line = 0;

// ===== Hardware Initialization Functions =====

static void init_buttons(void) {
    // Button A
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    
    // Button B
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);
    
    // Joystick Button
    gpio_init(JOYSTICK_BTN_PIN);
    gpio_set_dir(JOYSTICK_BTN_PIN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BTN_PIN);
    
    LOG_DEBUG("Botões inicializados (A:%d, B:%d, Joy:%d)", BTN_A_PIN, BTN_B_PIN, JOYSTICK_BTN_PIN);
}

static void init_adc(void) {
    adc_init();
    
    // Initialize ADC pins for joystick
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
    
    LOG_DEBUG("ADC inicializado (X:%d, Y:%d)", JOYSTICK_X_PIN, JOYSTICK_Y_PIN);
}

static void init_rgb_led(void) {
    // RGB LED uses PWM for brightness control
    // Note: BitDogLab has common cathode RGB, active HIGH
    
    gpio_set_function(LED_R_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_G_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_B_PIN, GPIO_FUNC_PWM);
    
    uint slice_r = pwm_gpio_to_slice_num(LED_R_PIN);
    uint slice_g = pwm_gpio_to_slice_num(LED_G_PIN);
    uint slice_b = pwm_gpio_to_slice_num(LED_B_PIN);
    
    pwm_set_wrap(slice_r, 255);
    pwm_set_wrap(slice_g, 255);
    pwm_set_wrap(slice_b, 255);
    
    pwm_set_enabled(slice_r, true);
    pwm_set_enabled(slice_g, true);
    pwm_set_enabled(slice_b, true);
    
    // Start with LED off (LOW for common cathode)
    pwm_set_gpio_level(LED_R_PIN, 0);
    pwm_set_gpio_level(LED_G_PIN, 0);
    pwm_set_gpio_level(LED_B_PIN, 0);
    
    LOG_DEBUG("LED RGB inicializado (R:%d, G:%d, B:%d)", LED_R_PIN, LED_G_PIN, LED_B_PIN);
}

static void set_rgb_led(uint8_t r, uint8_t g, uint8_t b) {
    // Direct PWM values for common cathode LED (higher value = brighter)
    pwm_set_gpio_level(LED_R_PIN, r);
    pwm_set_gpio_level(LED_G_PIN, g);
    pwm_set_gpio_level(LED_B_PIN, b);
    rgb_r = r;
    rgb_g = g;
    rgb_b = b;
}

static void read_inputs(void) {
    // Read buttons (active LOW)
    btn_a_pressed = !gpio_get(BTN_A_PIN);
    btn_b_pressed = !gpio_get(BTN_B_PIN);
    joy_btn_pressed = !gpio_get(JOYSTICK_BTN_PIN);
    
    // Read joystick ADC
    adc_select_input(JOYSTICK_X_ADC);
    joystick_x = adc_read();
    
    adc_select_input(JOYSTICK_Y_ADC);
    joystick_y = adc_read();
}

static void oled_push_line(const char *text) {
    // Shift all lines up
    for (int i = 0; i < OLED_MAX_LINES - 1; i++) {
        strncpy(oled_lines[i], oled_lines[i + 1], OLED_MAX_CHARS - 1);
        oled_lines[i][OLED_MAX_CHARS - 1] = '\0';
    }
    
    // Add new line at bottom
    strncpy(oled_lines[OLED_MAX_LINES - 1], text, OLED_MAX_CHARS - 1);
    oled_lines[OLED_MAX_LINES - 1][OLED_MAX_CHARS - 1] = '\0';
    
    // Update OLED display
    for (int i = 0; i < OLED_MAX_LINES; i++) {
        oled_set_text_line(i, oled_lines[i], OLED_ALIGN_LEFT);
    }
    oled_render_text();
    
    LOG_DEBUG("OLED: %s", text);
}

static void init_bitdoglab_matrix(void) {
    npInit(NEOPIXEL_PIN);
    LOG_DEBUG("Matriz LED BitDogLab inicializada (GPIO:%d, LEDs:%d)", NEOPIXEL_PIN, NEOPIXEL_NUM_LEDS);
}


static void init_bitdoglab_hardware(void) {
    LOG_INFO("Inicializando hardware BitDogLab...");
    
    init_buttons();
    init_adc();
    init_rgb_led();
    init_bitdoglab_matrix();
    
    // Initialize OLED
    oled_init();
    oled_clear();
    oled_set_text_line(0, "BitDogLab", OLED_ALIGN_CENTER);
    oled_set_text_line(1, "HTTP Server", OLED_ALIGN_CENTER);
    oled_set_text_line(3, "Conectando...", OLED_ALIGN_CENTER);
    oled_render_text();
    
    // Clear OLED lines buffer
    for (int i = 0; i < OLED_MAX_LINES; i++) {
        oled_lines[i][0] = '\0';
    }
    
    LOG_INFO("Hardware BitDogLab inicializado!");
}

#if LWIP_MDNS_RESPONDER
static void srv_txt(struct mdns_service *service, void *txt_userdata)
{
  err_t res;
  LWIP_UNUSED_ARG(txt_userdata);

  res = mdns_resp_add_service_txtitem(service, "path=/", 6);
  LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return);
}
#endif

// Return some characters from the ascii representation of the mac address
// e.g. 112233445566
// chr_off is index of character in mac to start
// chr_len is length of result
// chr_off=8 and chr_len=4 would return "5566"
// Return number of characters put into destination
static size_t get_mac_ascii(int idx, size_t chr_off, size_t chr_len, char *dest_in) {
    static const char hexchr[16] = "0123456789ABCDEF";
    uint8_t mac[6];
    char *dest = dest_in;
    assert(chr_off + chr_len <= (2 * sizeof(mac)));
    cyw43_hal_get_mac(idx, mac);
    for (; chr_len && (chr_off >> 1) < sizeof(mac); ++chr_off, --chr_len) {
        *dest++ = hexchr[mac[chr_off >> 1] >> (4 * (1 - (chr_off & 1))) & 0xf];
    }
    return dest - dest_in;
}

// ===== URL Decode and Text Processing =====

// Convert hex char to int (0-15)
static int hex_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

// Full URL decode in-place
static void url_decode(char *str) {
    char *src = str;
    char *dst = str;
    
    while (*src) {
        if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else if (*src == '%' && src[1] && src[2]) {
            int h1 = hex_to_int(src[1]);
            int h2 = hex_to_int(src[2]);
            if (h1 >= 0 && h2 >= 0) {
                *dst++ = (char)((h1 << 4) | h2);
                src += 3;
            } else {
                *dst++ = *src++;
            }
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

// Remove accents from UTF-8 text (convert to ASCII equivalent)
// Handles common Portuguese/Spanish accents
static void remove_accents(char *str) {
    unsigned char *src = (unsigned char *)str;
    char *dst = str;
    
    while (*src) {
        // UTF-8 two-byte sequences starting with 0xC3 (Latin-1 Supplement)
        if (*src == 0xC3 && src[1]) {
            unsigned char c = src[1];
            src += 2;
            
            // Map accented chars to ASCII equivalents
            switch (c) {
                // Lowercase
                case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: // àáâãäå
                    *dst++ = 'a'; break;
                case 0xA7: // ç
                    *dst++ = 'c'; break;
                case 0xA8: case 0xA9: case 0xAA: case 0xAB: // èéêë
                    *dst++ = 'e'; break;
                case 0xAC: case 0xAD: case 0xAE: case 0xAF: // ìíîï
                    *dst++ = 'i'; break;
                case 0xB1: // ñ
                    *dst++ = 'n'; break;
                case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: // òóôõö
                    *dst++ = 'o'; break;
                case 0xB9: case 0xBA: case 0xBB: case 0xBC: // ùúûü
                    *dst++ = 'u'; break;
                case 0xBD: // ý
                    *dst++ = 'y'; break;
                // Uppercase
                case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: // ÀÁÂÃÄÅ
                    *dst++ = 'A'; break;
                case 0x87: // Ç
                    *dst++ = 'C'; break;
                case 0x88: case 0x89: case 0x8A: case 0x8B: // ÈÉÊË
                    *dst++ = 'E'; break;
                case 0x8C: case 0x8D: case 0x8E: case 0x8F: // ÌÍÎÏ
                    *dst++ = 'I'; break;
                case 0x91: // Ñ
                    *dst++ = 'N'; break;
                case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: // ÒÓÔÕÖ
                    *dst++ = 'O'; break;
                case 0x99: case 0x9A: case 0x9B: case 0x9C: // ÙÚÛÜ
                    *dst++ = 'U'; break;
                case 0x9D: // Ý
                    *dst++ = 'Y'; break;
                default:
                    *dst++ = '?'; // Unknown char
                    break;
            }
        }
        // Skip other multi-byte UTF-8 sequences (3-4 bytes)
        else if ((*src & 0xE0) == 0xC0) { // 2-byte sequence
            src += 2;
            *dst++ = '?';
        }
        else if ((*src & 0xF0) == 0xE0) { // 3-byte sequence
            src += 3;
            *dst++ = '?';
        }
        else if ((*src & 0xF8) == 0xF0) { // 4-byte sequence
            src += 4;
            *dst++ = '?';
        }
        // ASCII printable range
        else if (*src >= 32 && *src < 127) {
            *dst++ = *src++;
        }
        // Skip other non-printable characters
        else {
            src++;
        }
    }
    *dst = '\0';
}

static const char *cgi_handler_index(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    return "/index.shtml";
}

// CGI handler for RGB LED control
static const char *cgi_handler_rgb(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    uint8_t r = 0, g = 0, b = 0;
    
    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "r") == 0) {
            r = (uint8_t)atoi(pcValue[i]);
        } else if (strcmp(pcParam[i], "g") == 0) {
            g = (uint8_t)atoi(pcValue[i]);
        } else if (strcmp(pcParam[i], "b") == 0) {
            b = (uint8_t)atoi(pcValue[i]);
        }
    }
    
    set_rgb_led(r, g, b);
    LOG_DEBUG("RGB LED: R=%d, G=%d, B=%d", r, g, b);
    
    return "/index.shtml";
}

// CGI handler for OLED text
static const char *cgi_handler_oled(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "text") == 0) {
            // Decode URL and remove accents for OLED display
            url_decode(pcValue[i]);
            remove_accents(pcValue[i]);
            pcValue[i][OLED_MAX_CHARS] = '\0';
            oled_push_line(pcValue[i]);
            break;
        }
    }
    return "/index.shtml";
}

// CGI handler for LED Matrix
static const char *cgi_handler_matrix(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "data") == 0) {
            // Parse comma-separated hex colors
            char *data = pcValue[i];
            int led_index = 0;
            char *token = strtok(data, ",");
            
            while (token != NULL && led_index < NEOPIXEL_NUM_LEDS) {
                // Convert hex string to uint32
                uint32_t color = (uint32_t)strtoul(token, NULL, 16);
                // Extract RGB components from hex
                uint8_t r = (color >> 16) & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t b = color & 0xFF;
                npSetLED(led_index, r, g, b);
                led_index++;
                
            }
            LOG_DEBUG("LED Matrix updated, %d LEDs", led_index);
            npWrite();
            break;
        }
    }
    return "/index.shtml";
}

static tCGI cgi_handlers[] = {
    { "/", cgi_handler_index },
    { "/index.shtml", cgi_handler_index },
    { "/rgb.cgi", cgi_handler_rgb },
    { "/oled.cgi", cgi_handler_oled },
    { "/matrix.cgi", cgi_handler_matrix },
};

// Note that the buffer size is limited by LWIP_HTTPD_MAX_TAG_INSERT_LEN, so use LWIP_HTTPD_SSI_MULTIPART to return larger amounts of data
u16_t ssi_example_ssi_handler(int iIndex, char *pcInsert, int iInsertLen
#if LWIP_HTTPD_SSI_MULTIPART
    , uint16_t current_tag_part, uint16_t *next_tag_part
#endif
) {
    size_t printed;
    
    // Read current input states before processing SSI
    read_inputs();
    
    switch (iIndex) {
        case 0: { // "status"
            printed = snprintf(pcInsert, iInsertLen, "Pass");
            break;
        }
        case 1: { // "welcome"
            printed = snprintf(pcInsert, iInsertLen, "Hello from Pico");
            break;
        }
        case 2: { // "uptime"
            uint64_t uptime_s = absolute_time_diff_us(wifi_connected_time, get_absolute_time()) / 1e6;
            printed = snprintf(pcInsert, iInsertLen, "%"PRIu64, uptime_s);
            break;
        }
        case 3: { // "ledstate"
            printed = snprintf(pcInsert, iInsertLen, "%s", led_on ? "ON" : "OFF");
            break;
        }
        case 4: { // "ledinv"
            printed = snprintf(pcInsert, iInsertLen, "%s", led_on ? "OFF" : "ON");
            break;
        }
#if LWIP_HTTPD_SSI_MULTIPART
        case 5: { /* "table" */
            printed = snprintf(pcInsert, iInsertLen, "<tr><td>This is table row number %d</td></tr>", current_tag_part + 1);
            // Leave "next_tag_part" unchanged to indicate that all data has been returned for this tag
            if (current_tag_part < 9) {
                *next_tag_part = current_tag_part + 1;
            }
            break;
        }
#endif
        case 6: { // "btna" - Button A state
            printed = snprintf(pcInsert, iInsertLen, "%d", btn_a_pressed ? 0 : 1);
            break;
        }
        case 7: { // "btnb" - Button B state
            printed = snprintf(pcInsert, iInsertLen, "%d", btn_b_pressed ? 0 : 1);
            break;
        }
        case 8: { // "joyx" - Joystick X axis
            printed = snprintf(pcInsert, iInsertLen, "%u", joystick_x);
            break;
        }
        case 9: { // "joyy" - Joystick Y axis
            printed = snprintf(pcInsert, iInsertLen, "%u", joystick_y);
            break;
        }
        case 10: { // "joybtn" - Joystick button
            printed = snprintf(pcInsert, iInsertLen, "%d", joy_btn_pressed ? 0 : 1);
            break;
        }
        case 11: { // "rgbr" - RGB Red value
            printed = snprintf(pcInsert, iInsertLen, "%u", rgb_r);
            break;
        }
        case 12: { // "rgbg" - RGB Green value
            printed = snprintf(pcInsert, iInsertLen, "%u", rgb_g);
            break;
        }
        case 13: { // "rgbb" - RGB Blue value
            printed = snprintf(pcInsert, iInsertLen, "%u", rgb_b);
            break;
        }
        default: { // unknown tag
            printed = 0;
            break;
        }
    }
  return (u16_t)printed;
}

// Be aware of LWIP_HTTPD_MAX_TAG_NAME_LEN
static const char *ssi_tags[] = {
    "status",   // 0
    "welcome",  // 1
    "uptime",   // 2
    "ledstate", // 3
    "ledinv",   // 4
    "table",    // 5
    "btna",     // 6
    "btnb",     // 7
    "joyx",     // 8
    "joyy",     // 9
    "joybtn",   // 10
    "rgbr",     // 11
    "rgbg",     // 12
    "rgbb",     // 13
};

#if LWIP_HTTPD_SUPPORT_POST
#define LED_STATE_BUFSIZE 4
static void *current_connection;

err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
        u16_t http_request_len, int content_len, char *response_uri,
        u16_t response_uri_len, u8_t *post_auto_wnd) {
    if (current_connection != connection) {
        if (memcmp(uri, "/led.cgi", 8) == 0 ||
            memcmp(uri, "/rgb.cgi", 8) == 0 ||
            memcmp(uri, "/oled.cgi", 9) == 0 ||
            memcmp(uri, "/matrix.cgi", 11) == 0) {
            current_connection = connection;
            snprintf(response_uri, response_uri_len, "/index.shtml");
            *post_auto_wnd = 1;
            return ERR_OK;
        }
    }
    return ERR_VAL;
}

// Return a value for a parameter
char *httpd_param_value(struct pbuf *p, const char *param_name, char *value_buf, size_t value_buf_len) {
    size_t param_len = strlen(param_name);
    u16_t param_pos = pbuf_memfind(p, param_name, param_len, 0);
    if (param_pos != 0xFFFF) {
        u16_t param_value_pos = param_pos + param_len;
        u16_t param_value_len = 0;
        u16_t tmp = pbuf_memfind(p, "&", 1, param_value_pos);
        if (tmp != 0xFFFF) {
            param_value_len = tmp - param_value_pos;
        } else {
            param_value_len = p->tot_len - param_value_pos;
        }
        if (param_value_len > 0 && param_value_len < value_buf_len) {
            char *result = (char *)pbuf_get_contiguous(p, value_buf, value_buf_len, param_value_len, param_value_pos);
            if (result) {
                result[param_value_len] = 0;
                return result;
            }
        }
    }
    return NULL;
}

#define POST_BUF_SIZE 512
static char post_buffer[POST_BUF_SIZE];

err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
    err_t ret = ERR_VAL;
    LWIP_ASSERT("NULL pbuf", p != NULL);
    
    if (current_connection == connection) {
        // Copy POST data to buffer
        u16_t len = pbuf_copy_partial(p, post_buffer, POST_BUF_SIZE - 1, 0);
        post_buffer[len] = '\0';
        
        // Handle LED state (legacy)
        char buf[8];
        char *val = httpd_param_value(p, "led_state=", buf, sizeof(buf));
        if (val) {
            led_on = (strcmp(val, "ON") == 0) ? true : false;
            cyw43_gpio_set(&cyw43_state, 0, led_on);
            ret = ERR_OK;
        }
        
        // Handle RGB LED
        char r_buf[8], g_buf[8], b_buf[8];
        char *r_val = httpd_param_value(p, "r=", r_buf, sizeof(r_buf));
        char *g_val = httpd_param_value(p, "g=", g_buf, sizeof(g_buf));
        char *b_val = httpd_param_value(p, "b=", b_buf, sizeof(b_buf));
        if (r_val && g_val && b_val) {
            set_rgb_led((uint8_t)atoi(r_val), (uint8_t)atoi(g_val), (uint8_t)atoi(b_val));
            ret = ERR_OK;
        }
        
        // Handle OLED text
        // Buffer larger to handle URL-encoded UTF-8 chars before decoding
        char text_buf[OLED_MAX_CHARS * 4 + 1];
        char *text_val = httpd_param_value(p, "text=", text_buf, sizeof(text_buf));
        if (text_val) {
            // Full URL decode (%XX and +)
            url_decode(text_val);
            // Remove accents (OLED doesn't support them)
            remove_accents(text_val);
            // Truncate to max OLED chars
            text_val[OLED_MAX_CHARS] = '\0';
            oled_push_line(text_val);
            ret = ERR_OK;
        }
        
        // Handle LED Matrix data
        char data_buf[256];
        char *data_val = httpd_param_value(p, "data=", data_buf, sizeof(data_buf));
        if (data_val) {
            // Parse comma-separated hex colors
            int led_index = 0;
            char *token = strtok(data_val, ",");
            while (token != NULL && led_index < NEOPIXEL_NUM_LEDS) {
                uint32_t color = (uint32_t)strtoul(token, NULL, 16);

                uint8_t r = (color >> 16) & 0xFF;
                uint8_t g = (color >> 8) & 0xFF;
                uint8_t b = color & 0xFF;
                npSetLED(led_index, r, g, b);

                led_index++;
                token = strtok(NULL, ",");
            }
            LOG_DEBUG("LED Matrix: %d LEDs updated", led_index);
            npWrite();
            ret = ERR_OK;
        }
    }
    
    pbuf_free(p);
    return ret;
}

void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
    snprintf(response_uri, response_uri_len, "/index.shtml");
    current_connection = NULL;
}
#endif

int main() {
    stdio_init_all();
    
    // Aguarda conexão USB ser estabelecida (importante para ver logs iniciais)
    sleep_ms(2000);
    
    // Configura nível de log em runtime (TRACE mostra tudo)
    log_set_level(LOG_LEVEL_TRACE);
    
    LOG_INFO("=== BitDogLab HTTP Server ===");
    LOG_DEBUG("Inicializando sistema...");
    
    // Initialize BitDogLab hardware first
    init_bitdoglab_hardware();
    
    if (cyw43_arch_init()) {
        LOG_WARN("Falha ao inicializar CYW43!");
        oled_set_text_line(3, "WiFi ERRO!", OLED_ALIGN_CENTER);
        oled_render_text();
        return 1;
    }
    LOG_DEBUG("CYW43 inicializado com sucesso");
    
    cyw43_arch_enable_sta_mode();
    LOG_TRACE("Modo STA habilitado");

    char hostname[sizeof(CYW43_HOST_NAME) + 4];
    memcpy(&hostname[0], CYW43_HOST_NAME, sizeof(CYW43_HOST_NAME) - 1);
    get_mac_ascii(CYW43_HAL_MAC_WLAN0, 8, 4, &hostname[sizeof(CYW43_HOST_NAME) - 1]);
    hostname[sizeof(hostname) - 1] = '\0';
    netif_set_hostname(&cyw43_state.netif[CYW43_ITF_STA], hostname);
    LOG_DEBUG("Hostname configurado: %s", hostname);

    LOG_INFO("Conectando ao WiFi: %s", WIFI_SSID);
    oled_set_text_line(2, WIFI_SSID, OLED_ALIGN_CENTER);
    oled_render_text();
    
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        LOG_WARN("Falha na conexao WiFi!");
        oled_set_text_line(3, "WiFi FALHOU!", OLED_ALIGN_CENTER);
        oled_render_text();
        exit(1);
    }
    LOG_INFO("WiFi conectado com sucesso!");
    
    const char *ip_str = ip4addr_ntoa(netif_ip4_addr(netif_list));
    LOG_INFO("Servidor HTTP disponivel em: %s", ip_str);
    
    // Update OLED with connection info
    oled_clear();
    oled_set_text_line(0, "BitDogLab", OLED_ALIGN_CENTER);
    oled_set_text_line(1, "HTTP Server", OLED_ALIGN_CENTER);
    oled_set_text_line(3, "Conectado!", OLED_ALIGN_CENTER);
    oled_set_text_line(5, ip_str, OLED_ALIGN_CENTER);
    oled_render_text();

    // start http server
    wifi_connected_time = get_absolute_time();

#if LWIP_MDNS_RESPONDER
    // Setup mdns
    LOG_DEBUG("Configurando mDNS...");
    cyw43_arch_lwip_begin();
    mdns_resp_init();
    LOG_INFO("mDNS hostname: %s.local", hostname);
#if LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 2
    mdns_resp_add_netif(&cyw43_state.netif[CYW43_ITF_STA], hostname);
    mdns_resp_add_service(&cyw43_state.netif[CYW43_ITF_STA], "pico_httpd", "_http", DNSSD_PROTO_TCP, 80, srv_txt, NULL);
#else
    mdns_resp_add_netif(&cyw43_state.netif[CYW43_ITF_STA], hostname, 60);
    mdns_resp_add_service(&cyw43_state.netif[CYW43_ITF_STA], "pico_httpd", "_http", DNSSD_PROTO_TCP, 80, 60, srv_txt, NULL);
#endif
    cyw43_arch_lwip_end();
    LOG_TRACE("mDNS configurado");
#endif
    // setup http server
    LOG_DEBUG("Inicializando servidor HTTP...");
    cyw43_arch_lwip_begin();
    httpd_init();
    http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
    http_set_ssi_handler(ssi_example_ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
    cyw43_arch_lwip_end();
    LOG_INFO("Servidor HTTP iniciado!");

    LOG_INFO("Entrando no loop principal...");
    uint32_t loop_count = 0;
    while(true) {
#if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(led_time);
#else
        sleep_ms(1000);
        loop_count++;
        // Log periódico a cada 30 segundos para mostrar que está ativo
        if (loop_count % 30 == 0) {
            LOG_TRACE("Sistema ativo - uptime: %lu segundos", loop_count);
        }
#endif
    }
#if LWIP_MDNS_RESPONDER
    mdns_resp_remove_netif(&cyw43_state.netif[CYW43_ITF_STA]);
#endif
    cyw43_arch_deinit();
}
