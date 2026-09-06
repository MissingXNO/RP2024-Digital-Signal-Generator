cmake_minimum_required(VERSION 3.13)

# Always include it
include(pico_sdk_import.cmake)

# Project's name
project(signal_gen)

# SDK Initialization - Mandatory
pico_sdk_init()

# C/C++ project files
add_executable(main
    main.c
)

# pico_stdlib library. You can add more if they are needed
target_link_libraries(main pico_stdlib hardware_pwm)

# Enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# Need to generate UF2 file for upload to RP2040
pico_add_extra_outputs(main)