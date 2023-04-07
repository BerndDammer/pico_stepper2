# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
# set(PICO_SDK_PATH "d:/RaspberryPiPico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(stepper2 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1


add_executable(stepper2 main.c )

pico_set_program_name(stepper2 "stepper2")
pico_set_program_version(stepper2 "0.2")

pico_enable_stdio_uart(stepper2 1)
pico_enable_stdio_usb(stepper2 1)

target_sources(stepper2 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/timer.c
        ${CMAKE_CURRENT_LIST_DIR}/loop.c
        ${CMAKE_CURRENT_LIST_DIR}/loop_main.c
        ${CMAKE_CURRENT_LIST_DIR}/loop_bitbang.c
        ${CMAKE_CURRENT_LIST_DIR}/loop_single.c
        ${CMAKE_CURRENT_LIST_DIR}/loop_encoder.c
        ${CMAKE_CURRENT_LIST_DIR}/blinker.c
        ${CMAKE_CURRENT_LIST_DIR}/motor.c
        ${CMAKE_CURRENT_LIST_DIR}/motor_single.c
        ${CMAKE_CURRENT_LIST_DIR}/encoder.c
        )

# Add the standard library to the build
target_link_libraries(stepper2 
	pico_stdlib
        hardware_timer
        hardware_watchdog
        hardware_clocks
	hardware_pwm
)


