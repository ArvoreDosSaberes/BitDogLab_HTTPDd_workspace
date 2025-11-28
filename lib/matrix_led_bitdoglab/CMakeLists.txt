# WS2818B PIO-based LED driver library

add_library(bitdog_lab_matrix_led INTERFACE)

# Generate the PIO header
pico_generate_pio_header(bitdog_lab_matrix_led ${CMAKE_CURRENT_LIST_DIR}/ws2818b.pio)

target_sources(bitdog_lab_matrix_led INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/neopixel_pio.c
)

target_include_directories(bitdog_lab_matrix_led INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(bitdog_lab_matrix_led INTERFACE
    pico_stdlib
    hardware_pio
    hardware_clocks
)
