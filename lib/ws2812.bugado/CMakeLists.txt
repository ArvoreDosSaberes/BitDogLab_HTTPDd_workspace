# WS2812 PIO-based LED driver library

add_library(ws2812 INTERFACE)

# Generate the PIO header
pico_generate_pio_header(ws2812 ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_sources(ws2812 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ws2812.c
)

target_include_directories(ws2812 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(ws2812 INTERFACE
    pico_stdlib
    hardware_pio
    hardware_clocks
)
