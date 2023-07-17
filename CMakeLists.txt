cmake_minimum_required(VERSION 3.20)
set(FREERTOS_PORT GCC_RP2040)
set(PICO_BOARD pico_w)
include(vendor/pico-sdk/pico_sdk_init.cmake)
include(vendor/free-rtos/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(pcroast
    VERSION 0.0.1.0
    DESCRIPTION "Control for Reflow Ovens"
    HOMEPAGE_URL https://github.com/zoftko/pcroast
    LANGUAGES C CXX ASM
)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions("DEBUG_BUILD=$<CONFIG:Debug>")
add_compile_definitions("PROJECT_NAME=\"${CMAKE_PROJECT_NAME}\"")
add_compile_definitions("PROJECT_VERSION=\"${CMAKE_PROJECT_VERSION}\"")

if (NOT WIFI_TIMEOUT)
    set(WIFI_TIMEOUT 10000)
endif ()

add_compile_definitions("WIFI_TIMEOUT=${WIFI_TIMEOUT}")
add_compile_definitions("WIFI_SSID=\"${WIFI_SSID}\"")
add_compile_definitions("WIFI_PSK=\"${WIFI_PSK}\"")


pico_sdk_init()

include_directories(pico)
include_directories(include)

add_subdirectory(pico)
