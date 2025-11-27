# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(picow_httpd C CXX ASM)

set(WIFI_SSID "TP-Link_64E1")
set(WIFI_PASSWORD "85336131")

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Adiciona a biblioteca de logging com cores VT100
add_subdirectory(lib)

# Add executable. Default name is the project name, version 0.1

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR})

add_executable(picow_httpd_background
        pico_httpd.c
        )
target_compile_definitions(picow_httpd_background PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )
target_include_directories(picow_httpd_background PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        ${CMAKE_CURRENT_LIST_DIR}/lib/OLED_SSD1306
        ${PICO_LWIP_CONTRIB_PATH}/apps/httpd
        )
target_link_libraries(picow_httpd_background PRIVATE
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_http
        pico_lwip_mdns
        pico_httpd_content
        pico_stdlib
        
        hardware_adc
        hardware_pwm
        hardware_i2c
        hardware_pio

        i2c_proxy

        log_vt100
        oled
        ws2812
        )

# Habilita saída USB (stdio via USB) e desabilita UART
pico_enable_stdio_usb(picow_httpd_background 1)
pico_enable_stdio_uart(picow_httpd_background 0)

pico_add_extra_outputs(picow_httpd_background)

pico_add_library(pico_httpd_content NOFLAG)
pico_set_lwip_httpd_content(pico_httpd_content INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/content/404.html
        ${CMAKE_CURRENT_LIST_DIR}/content/index.shtml
        ${CMAKE_CURRENT_LIST_DIR}/content/state.shtml
        ${CMAKE_CURRENT_LIST_DIR}/content/css/style.css.gz
        ${CMAKE_CURRENT_LIST_DIR}/content/js/app.js.gz
        ${CMAKE_CURRENT_LIST_DIR}/content/img/rpi.png.gz
        )
