cmake_minimum_required(VERSION 3.20)
set(FREERTOS_PORT GCC_RP2040)
include(vendor/pico-sdk/pico_sdk_init.cmake)

project(pcroast
    VERSION 0.0.1
    DESCRIPTION "Control for Reflow Ovens"
    HOMEPAGE_URL https://github.com/zoftko/pcroast
    LANGUAGES C CXX ASM
)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
add_subdirectory(pico)
